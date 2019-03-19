#include "mainwindow.h"

#include <QLabel>
#include <QNetworkInterface>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

MainWindow::MainWindow()
{
  QGridLayout* mainLayout = new QGridLayout();
  mainLayout->addWidget(createNetworkStatusGroupBox(), 0, 0, 1, 1);
  mainLayout->addWidget(createQrCodeGroupBox(), 0, 1, 1, 1);

  setLayout(mainLayout);
  setWindowTitle("EasyConnect Demo");

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &MainWindow::checkForInterfaceChanges);
  m_timer->setInterval(500);
  m_timer->start();
}

MainWindow::~MainWindow()
{
}

QWidget*
MainWindow::createQrCodeGroupBox()
{
}

QWidget*
MainWindow::createNetworkStatusGroupBox()
{
  QVBoxLayout* vbox = new QVBoxLayout();

  for (QNetworkInterface const& nic : QNetworkInterface::allInterfaces())
  {
    int row = 0;

    QGridLayout* gridLayout = new QGridLayout();
    QString name = nic.name();
    int index = nic.index();

    gridLayout->addWidget(new QLabel("Name:"), row, 0, 1, 1);
    gridLayout->addWidget(new QLabel(nic.name()), row, 1, 1, 1);
    row++;

    bool hasAddress = false;
    QLabel* address = nullptr;
    QLabel* netmask = nullptr;

    for (QNetworkAddressEntry const& addr : nic.addressEntries())
    {
      address = new QLabel(addr.ip().toString());
      netmask = new QLabel(addr.netmask().toString());

      gridLayout->addWidget(new QLabel("Address:"), row, 0, 1, 1);
      gridLayout->addWidget(address, row, 1, 1, 1);
      row++;

      gridLayout->addWidget(new QLabel("Mask:"), row, 0, 1, 1);
      gridLayout->addWidget(netmask, row, 1, 1, 1);
      row++;

      hasAddress = true;
    }

    if (!hasAddress)
    {
      address = new QLabel("");
      netmask = new QLabel("");

      gridLayout->addWidget(new QLabel("Address:"), row, 0, 1, 1);
      gridLayout->addWidget(address, row, 1, 1, 1);
      row++;

      gridLayout->addWidget(new QLabel("Mask:"), row, 0, 1, 1);
      gridLayout->addWidget(netmask, row, 1, 1, 1);
      row++;

      m_wirelessInterfaces.append(index);

      connect(this, &MainWindow::networkAddressChanged, [this, address, netmask, index](int i)
      {
        if (index != i)
          return;

        QNetworkInterface networkInterface = QNetworkInterface::interfaceFromIndex(index);
        for (QNetworkAddressEntry entry : networkInterface.addressEntries())
        {
          if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
          {
            address->setText(entry.ip().toString());
            netmask->setText(entry.netmask().toString());
            m_wirelessInterfaces.removeAll(index);
            return;
          }
        }
      });
    }

    QGroupBox* groupBox = new QGroupBox(nic.name());
    groupBox->setLayout(gridLayout);

    vbox->addWidget(groupBox);
  }

  QGroupBox* groupBox = new QGroupBox("Network Information");
  groupBox->setLayout(vbox);

  return groupBox;
}

void
MainWindow::checkForInterfaceChanges()
{
  for (int index : m_wirelessInterfaces)
  {
    QNetworkInterface networkInterface = QNetworkInterface::interfaceFromIndex(index);
    QList<QNetworkAddressEntry> addresses = networkInterface.addressEntries();

    // only handles cases where there was no address when started, and now there is
    if (addresses.size() > 0)
    {
      qInfo() << "changed";
      emit networkAddressChanged(index);
    }
  }
}
