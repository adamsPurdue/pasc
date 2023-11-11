#pragma once


#ifndef PASC_H
#define PASC_H

#include <Qt>

#include <cmath>

#include <math.h>
#include <algorithm>
//#include <QtDataVisualization/>
#include <q3dscatter.h>
#include <q3dsurface.h>
#include <q3dbars.h>

#include <QtWidgets/QMainWindow>
//#include <qmainwindow.h>
#include <qwidget.h>
#include <qserialport.h>
#include <qserialportinfo.h>
//#include <qcombobox.h>
//#include <qpushbutton.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdatastream.h>
#include <qmessagebox.h>

#include <qlineseries.h>
#include <qelapsedtimer.h>

#include "ui_pasc.h"

//using namespace QtDataVisualization;
using namespace std;
//using namespace FUTEK_USB_DLL;

class pasc : public QMainWindow
{
	Q_OBJECT

public:
	pasc(QWidget* parent = Q_NULLPTR);
	~pasc();
	void initUI();

	QStringList getPortNameList(); // obtaining available port numbers
	void refreshPortNameList();

	void openPort(); // open the port
	void openPortSensor();
	void openPortElectrometer();
	void openPortFutek();

	void sendMessage(QString message, bool inLoop);
	QByteArray readMessage();
	void pcResponse(QString message);
	void edResponse(QString message);
	void readEDduringTravel();
	void lcResponse(QString message);
	void emResponse(QString message);
	void ftResponse(QString message);

	void singleReadXYZE();
	void setCurrentXYZE();

	void incrementXP();
	void incrementYP();
	void incrementZP();
	void incrementEP();
	void incrementXM();
	void incrementYM();
	void incrementZM();
	void incrementEM();

	void homeXWhenSafe();
	void homeYWhenSafe();
	void homeZWhenSafe();

	void disableEndStop();

	void zeroFutek();

	// Particular Functions
	void sendFromLineEdit();
	void sendCurrentProfile();
	void haltCurrentProfile();
	void composeHeader();
	void composeLinearScan();
	void composeCompression();
	void composeTimeElapse();
	void composeChargeCollect();
	void composeZProbe();
	void composeFormalCT();
	void composeKirigami();
	void composeCTFutek();

	void composeHeaderVoltage();
	void composeHeaderCharge();
	void composeHeaderChargeReady();

	// Save and load profiles
	void saveCurrentProfile();
	void loadProfile();
	void saveCurData();

	void lcDubug();
	void stopReading();
	//void appendToPlot();

protected:
	void timerEvent(QTimerEvent* event);

public slots:
	void reveiveInfo();
	//void sendMessage();

private:
	Ui::pascClass ui;

	QElapsedTimer timeStampTimer;
	int timer_Read1;
	bool profileHalted;
	int curProfileIndex;
	// 1-LinearScan // 2-Compression // 3-TimeElapse // 4-ChargeCollect // 5-ZProbe // 6-FormalCT // 7-Kirigami // 8-CTFutek

	int timer_LC1;
	int timer_ED1;

	bool sampleTimeOn;
	bool isLoopTiming;
	double loopTimingLimit;

	bool jogReclickLock;

	QSerialPort* ed_serialPort; // the Ender serial port
	QSerialPort* lc_serialPort; // the Load Cell port
	QSerialPort* em_serialPort; // the Electrometer port
	QSerialPort* mk_serialPort; // the Mark10 serial port
	QSerialPort* ft_serialPort; // the Futek serial port (listen only)


	QStringList ed_portNameList;
	QStringList ed_currentGCode;
	QString ed_currentMessage;
	QString ed_currentFeedback;
	QString lc_currentFeedback;
	QString pc_currentFeedback;
	QString em_currentFeedback;
	QString ft_currentFeedback;
	QByteArray ed_CurEndCode;
	int curCodeIndex;

	//const QString edPrefix = QString("ED: ");
	//const QString lcPrefix = QString("LC: ");
	//const QString emPrefix = QString("EM: ");
	const QStringList serialPrefixes = QStringList({ "ED: ", "LC: " ,"EM: ","PC: " });

	//QComboBox* ed_PortNameComboBox; // select the port
	//QPushButton* ed_OpenPortButton; // click to open selected port

	// Plot Initializations
	QVector<double> curXYZE;
	double curTimeReading;
	double curForceReading;
	double curElecReading;
	double curFutekReading;
	double curPressureReading;

	QVector<double> data_Time;
	QVector<double> data_Time_Range;
	QVector<double> data_Force;
	QVector<double> data_Force_Range;
	QVector<double> data_Elec;
	QVector<double> data_Elec_Range;
	QVector<double> data_X;
	QVector<double> data_X_Range;
	QVector<double> data_Y;
	QVector<double> data_Y_Range;
	QVector<double> data_Z;
	QVector<double> data_Z_Range;
	QVector<double> data_E;
	QVector<double> data_E_Range;
	QVector<double> data_Futek;
	QVector<double> data_Futek_Range;
	QVector<double> data_Pressure;
	QVector<double> data_Pressure_Range;



	QLineSeries* series_Plot1;
	QChart* chart_Plot1;

	QLineSeries* series_Plot2;
	QChart* chart_Plot2;



	QScatterDataItem* dataItem_Scatter_Plot1;
	QScatterDataProxy* proxy_Scatter_Plot1;
	QScatterDataArray* array_Scatter_Plot1;
	Q3DScatter* scatter_Plot1;
	QScatter3DSeries* series_Scatter_Plot1;
	QWidget* container_Scatter;

	Q3DSurface* surface_Plot1;
	QWidget* container_Surface;
	QSurfaceDataArray* array_Surface_Plot1;
	QSurfaceDataProxy* proxy_Surface_Plot1;
	QSurface3DSeries* series_Surface_Plot1;

	Q3DSurface* surface_Plot2;
	QWidget* container_Surface2;
	QSurfaceDataArray* array_Surface_Plot2;
	QSurfaceDataProxy* proxy_Surface_Plot2;
	QSurface3DSeries* series_Surface_Plot2;

	Q3DBars* bar_Plot1;
	QWidget* container_Bar;
	QBarDataArray* array_Bar_Plot1;
	QBarDataProxy* proxy_Bar_Plot1;
	QBar3DSeries* series_Bar_Plot1;

	//QScatterSeries
	//Q3DScatter scatter;

};

#endif // PASC_H