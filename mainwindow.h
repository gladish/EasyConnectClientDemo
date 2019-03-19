#ifndef __MAIN_WINDOW_H__
#define __MAIN_WINDOW_H__

#include <QDialog>
#include <QGroupBox>

QT_FORWARD_DECLARE_CLASS(QTimer);

class MainWindow : public QDialog
{
  Q_OBJECT

public:
  explicit MainWindow();
  ~MainWindow();

signals:
  void networkAddressChanged(int index);

private:
  QWidget* createNetworkStatusGroupBox();
  QWidget* createQrCodeGroupBox();

  void checkForInterfaceChanges();

private:
  QTimer*         m_timer;
  QList<int>      m_wirelessInterfaces;
};

#endif
