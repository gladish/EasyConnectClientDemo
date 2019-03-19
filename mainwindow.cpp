#include "mainwindow.h"

#include <QComboBox>
#include <QDateTime>
#include <QLabel>
#include <QNetworkInterface>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QPushButton>
#include <QProcess>

namespace
{
  QString const wpa_cli = "/home/pi/work/hostap/wpa_supplicant/wpa_cli";
  QString wpaCliGenerateUri(QString const& mac, QString const& channel)
  {
    QString bootstrapGen = QString("sudo %1 dpp_bootstrap_gen type=qrcode chan=81/%2 mac=%3")
      .arg(wpa_cli)
      .arg(channel)
      .arg(mac);

    qInfo() << "exec:" << bootstrapGen;
    QProcess p1;
    p1.start(bootstrapGen);
    p1.waitForFinished();

    QString out1 = p1.readAllStandardOutput();
    QStringList tokens = out1.split("\n");
    QString index = tokens[1];

    QString bootstrapGetUri = QString("sudo %1 dpp_bootstrap_get_uri %2")
      .arg(wpa_cli)
      .arg(index);

    qInfo() << "exec:" << bootstrapGetUri;
    QProcess p2;
    p2.start(bootstrapGetUri);
    p2.waitForFinished();

    QString out2 = p2.readAllStandardOutput();
    tokens = out2.split("\n");

    QString uri = tokens[1].remove('"');
    qInfo() << "dpp_uri:" << uri;
    return uri;
  }

  QString
  getWpaStatus()
  {
    QString status = QString("sudo %1 status")
      .arg(wpa_cli);

    QProcess p;
    p.start(status);
    p.waitForFinished();

    QString s = p.readAllStandardOutput();
    return s;
  }

  void
  regenerateQrCodePixmap(QString const& dppUri, QString const& path)
  {
    QString qrencode = QString("qrencode \"%1\" -o %2")
      .arg(dppUri)
      .arg(path);

    qInfo() << "exec:" << qrencode;

    QProcess p;
    p.start(qrencode);
    p.waitForFinished();
  }

  void
  startDppListen()
  {
    qInfo() << "TODO startDppListen";
  }
}

MainWindow::MainWindow()
{
  QGridLayout* mainLayout = new QGridLayout();
  mainLayout->addWidget(createNetworkStatusGroupBox(), 0, 0, 3, 1);
  mainLayout->addWidget(createQrCodeGroupBox(), 0, 1, 1, 1);
  mainLayout->addWidget(createDppStatusGroupBox(), 1, 1, 1, 1);
  mainLayout->addWidget(createWpaSupplicantStatus(), 2, 1, 1, 1);

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
MainWindow::createDppStatusGroupBox()
{
  QGroupBox* groupBox = new QGroupBox("EasyConnect Status");

  // TODO: 
  return groupBox;
}

QWidget*
MainWindow::createWpaSupplicantStatus()
{
  QGroupBox* groupBox = new QGroupBox("WPA Supplicant Status");
  QVBoxLayout* layout = new QVBoxLayout();

  QLabel* label = new QLabel();
  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [label]()
  {
    QDateTime now = QDateTime::currentDateTime();
    QString message = QString("%1\n%2")
      .arg(now.toString())
      .arg(getWpaStatus());

    label->setText(message);
  });
  layout->addWidget(label);
  groupBox->setLayout(layout);
  timer->start(1000);
  return groupBox;
}

QWidget*
MainWindow::createQrCodeGroupBox()
{
  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->addWidget(new QLabel("Wireless Interface"), 0, 0, 1, 1);

  QComboBox* comboBox = new QComboBox();
  comboBox->addItem("wlan0");
  comboBox->addItem("wlan1");
  gridLayout->addWidget(comboBox, 0, 1, 1, 1);

  gridLayout->addWidget(new QLabel("Channel"), 0, 2, 1, 1);

  QComboBox* channel = new QComboBox();
  for (int i = 1; i < 15; ++i)
    channel->addItem(QString::number(i));
  gridLayout->addWidget(channel, 0, 3, 1, 1);

  QLabel* qrCode = new QLabel();

  QPushButton* button = new QPushButton("Update QR Code");
  connect(button, &QPushButton::released, this, [this, qrCode, comboBox, channel]()
  {
    QString wirelessInterfaceName = comboBox->currentText();
    QString wifiChannel = channel->currentText();
    QString mac = QNetworkInterface::interfaceFromName(wirelessInterfaceName).hardwareAddress();
    QString dppUri = wpaCliGenerateUri(mac, wifiChannel);

    QString pathToPixmap("/tmp/qrcode.png");
    regenerateQrCodePixmap(dppUri, pathToPixmap);

    QPixmap image(pathToPixmap);
    qrCode->setPixmap(image);
  });
  gridLayout->addWidget(button, 1, 0, 1, 1);

  QPushButton* listenButton = new QPushButton("Start DPP Listen");
  connect(listenButton, &QPushButton::released, this, [this]()
  {
    startDppListen();
  });
  gridLayout->addWidget(listenButton, 1, 1, 1, 1);

  QPixmap image("qrcode.png");
  qrCode->setPixmap(image);
  gridLayout->addWidget(qrCode, 2, 0, 2, 2);

  QGroupBox* groupBox = new QGroupBox("QR Code");
  groupBox->setLayout(gridLayout);

  return groupBox;
}

QWidget*
MainWindow::createNetworkStatusGroupBox()
{
  QVBoxLayout* vbox = new QVBoxLayout();

  for (QNetworkInterface const& nic : QNetworkInterface::allInterfaces())
  {
    QString name = nic.name();
    if (name == "lo")
      continue;

    int row = 0;
    int index = nic.index();
    QGridLayout* gridLayout = new QGridLayout();

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
      emit networkAddressChanged(index);
  }
}
