#include "pasc.h"
#include <qlayout.h>
#include <qdebug.h>

pasc::pasc(QWidget* parent)
	: QMainWindow(parent)
	//	ui(new Ui::pasc)
{
	ui.setupUi(this);
	ed_serialPort = new QSerialPort();
	lc_serialPort = new QSerialPort();
	em_serialPort = new QSerialPort();
	ft_serialPort = new QSerialPort();
	ed_currentGCode = QStringList({ "M302 S0", "M107" });
	ed_currentMessage = "";
	curCodeIndex = 0;
	ed_currentFeedback = "Hello Ender";
	pc_currentFeedback = "Hello G7";
	lc_currentFeedback = "Hello Load Cell";
	em_currentFeedback = "Hello Keithley";
	ft_currentFeedback = "Hello Futek";
	sampleTimeOn = false;
	isLoopTiming = false;
	jogReclickLock = false;
	loopTimingLimit = 0;
	curXYZE = { 0,0,0,0 };
	curFutekReading = 0;
	initUI();

	QObject::connect(ui.button_ConnectPort_Ender, &QPushButton::clicked, this, &pasc::openPort);
	QObject::connect(ui.button_ConnectPort_LoadCell, &QPushButton::clicked, this, &pasc::openPortSensor);
	QObject::connect(ui.button_ConnectPort_Electrometer, &QPushButton::clicked, this, &pasc::openPortElectrometer);
	QObject::connect(ui.button_ConnectPort_Futek, &QPushButton::clicked, this, &pasc::openPortFutek);

	QObject::connect(ui.button_FutekZero, &QPushButton::clicked, this, &pasc::zeroFutek);

	QObject::connect(ui.button_SendMessage, &QPushButton::clicked, this, &pasc::sendFromLineEdit);
	//QObject::connect(ed_serialPort, &QSerialPort::readyRead, this, &pasc::readEDduringTravel);
	QObject::connect(ui.button_RefreshPortNameList, &QPushButton::clicked, this, &pasc::refreshPortNameList);
	QObject::connect(ui.button_ClearReadHistory, &QPushButton::clicked, ui.textBrowser_ReadHistory, &QTextBrowser::clear);
	QObject::connect(ui.button_ClearSendHistory, &QPushButton::clicked, ui.textBrowser_SendHistory, &QTextBrowser::clear);

	QObject::connect(ui.button_ComposeHeader, &QPushButton::clicked, this, &pasc::composeHeader);
	QObject::connect(ui.button_ComposeLinearScan, &QPushButton::clicked, this, &pasc::composeLinearScan);
	QObject::connect(ui.button_ComposeCompression, &QPushButton::clicked, this, &pasc::composeCompression);
	QObject::connect(ui.button_ComposeTimeElapse, &QPushButton::clicked, this, &pasc::composeTimeElapse);
	QObject::connect(ui.button_ComposeChargeCollect, &QPushButton::clicked, this, &pasc::composeChargeCollect);
	QObject::connect(ui.button_ComposeZProbe, &QPushButton::clicked, this, &pasc::composeZProbe);
	QObject::connect(ui.button_ComposeFormalCT, &QPushButton::clicked, this, &pasc::composeFormalCT);
	QObject::connect(ui.button_ComposeKirigami, &QPushButton::clicked, this, &pasc::composeKirigami);
	QObject::connect(ui.button_ComposeCTFutek, &QPushButton::clicked, this, &pasc::composeCTFutek);

	QObject::connect(ui.button_DisableEndStop, &QPushButton::clicked, this, &pasc::disableEndStop);

	QObject::connect(ui.button_SaveProfile, &QPushButton::clicked, this, &pasc::saveCurrentProfile);
	QObject::connect(ui.button_LoadProfile, &QPushButton::clicked, this, &pasc::loadProfile);
	QObject::connect(ui.button_SaveData, &QPushButton::clicked, this, &pasc::saveCurData);
	QObject::connect(ui.button_SendProfile, &QPushButton::clicked, this, &pasc::sendCurrentProfile);
	QObject::connect(ui.button_HaltProfile, &QPushButton::clicked, this, &pasc::haltCurrentProfile);

	QObject::connect(ui.button_AcquireXYZE, &QPushButton::clicked, this, &pasc::singleReadXYZE);
	QObject::connect(ui.button_SetXYZE, &QPushButton::clicked, this, &pasc::setCurrentXYZE);

	QObject::connect(ui.button_XP, &QPushButton::clicked, this, &pasc::incrementXP);
	QObject::connect(ui.button_YP, &QPushButton::clicked, this, &pasc::incrementYP);
	QObject::connect(ui.button_ZP, &QPushButton::clicked, this, &pasc::incrementZP);
	QObject::connect(ui.button_EP, &QPushButton::clicked, this, &pasc::incrementEP);
	QObject::connect(ui.button_XM, &QPushButton::clicked, this, &pasc::incrementXM);
	QObject::connect(ui.button_YM, &QPushButton::clicked, this, &pasc::incrementYM);
	QObject::connect(ui.button_ZM, &QPushButton::clicked, this, &pasc::incrementZM);
	QObject::connect(ui.button_EM, &QPushButton::clicked, this, &pasc::incrementEM);

	QObject::connect(ui.button_XHome, &QPushButton::clicked, this, &pasc::homeXWhenSafe);
	QObject::connect(ui.button_YHome, &QPushButton::clicked, this, &pasc::homeYWhenSafe);
	QObject::connect(ui.button_ZHome, &QPushButton::clicked, this, &pasc::homeZWhenSafe);
	//QObject::connect(ui.button_LoopDebug, &QPushButton::clicked, this, &pasc::loopDebug);
	//QObject::connect(ui.button_LoopStop, &QPushButton::clicked, this, &pasc::stopReading);
	QObject::connect(ui.button_EMDebug, &QPushButton::clicked, this, &pasc::lcDubug);


	// Preparing 2D Data Visualization
	this->series_Plot1 = new QLineSeries();
	this->chart_Plot1 = new QChart();
	this->chart_Plot1->legend()->hide();
	this->chart_Plot1->addSeries(this->series_Plot1);
	this->chart_Plot1->createDefaultAxes();
	this->chart_Plot1->setTitle("Test");
	ui.graphicsView_Debug->setChart(this->chart_Plot1);
	ui.graphicsView_Debug->setRenderHint(QPainter::Antialiasing);

	// Preparing 2D Data Visualization 2nd
	this->series_Plot2 = new QLineSeries();
	this->chart_Plot2 = new QChart();
	this->chart_Plot2->legend()->hide();
	this->chart_Plot2->addSeries(this->series_Plot2);
	this->chart_Plot2->createDefaultAxes();
	this->chart_Plot2->setTitle("Test");
	ui.graphicsView_Debug_2->setChart(this->chart_Plot2);
	ui.graphicsView_Debug_2->setRenderHint(QPainter::Antialiasing);

	// Preparing 3D Scatter
	this->proxy_Scatter_Plot1 = new QScatterDataProxy;
	this->array_Scatter_Plot1 = new QScatterDataArray;
	this->scatter_Plot1 = new Q3DScatter();
	this->array_Scatter_Plot1->reserve(1);
	//for (int i = 0; i < 1; i++) {
	//	array_Scatter_Plot1->append(QScatterDataItem(QVector3D(i, i, i)));
	//}
	this->proxy_Scatter_Plot1->resetArray(this->array_Scatter_Plot1);
	//this->scatter_Plot1 = new Q3DScatter();
	this->series_Scatter_Plot1 = new QScatter3DSeries(this->proxy_Scatter_Plot1);
	this->series_Scatter_Plot1->setMesh(QAbstract3DSeries::MeshPoint);
	this->series_Scatter_Plot1->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
	this->scatter_Plot1->addSeries(this->series_Scatter_Plot1);
	this->scatter_Plot1->activeTheme()->setType(Q3DTheme::ThemeStoneMoss);
	this->scatter_Plot1->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
	this->scatter_Plot1->setFlags(this->scatter_Plot1->flags() ^ Qt::FramelessWindowHint);
	//this->scatter_Plot1->activeTheme()->setBackgroundColor(QColor(0, 0, 0, 0));
	//this->scatter_Plot1->activeTheme()->setBackgroundEnabled(0);
	//this->scatter_Plot1->activeTheme()->setWindowColor(QColor(0, 0, 0, 0));
	//this->scatter_Plot1->activeTheme()->setLabelBackgroundColor(QColor(0, 0, 0, 255));
	this->container_Scatter = QWidget::createWindowContainer(this->scatter_Plot1);
	//this->container_Scatter->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255)"));
	//ui.verticalLayout_Container->addWidget(this->container_Scatter);
	//this->container_Scatter->show();
	this->scatter_Plot1->axisX()->setAutoAdjustRange(true);
	this->scatter_Plot1->axisY()->setAutoAdjustRange(true);
	this->scatter_Plot1->axisZ()->setAutoAdjustRange(true);

	// Preparing 3D Bar
	this->array_Bar_Plot1 = new QBarDataArray;
	this->proxy_Bar_Plot1 = new QBarDataProxy;
	//this->proxy_Bar_Plot1->resetArray(this->array_Bar_Plot1);
	this->series_Bar_Plot1 = new QBar3DSeries(this->proxy_Bar_Plot1);
	this->bar_Plot1 = new Q3DBars();
	this->bar_Plot1->addSeries(this->series_Bar_Plot1);
	this->container_Bar = QWidget::createWindowContainer(this->bar_Plot1);
	//this->bar_Plot1->rowAxis()->setAutoAdjustRange(true);
	//this->bar_Plot1->columnAxis()->setAutoAdjustRange(true);
	//this->bar_Plot1->valueAxis()->setAutoAdjustRange(true);
	this->bar_Plot1->setFlags(this->scatter_Plot1->flags() ^ Qt::FramelessWindowHint);
	this->bar_Plot1->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
	this->bar_Plot1->setBarSpacing(QSizeF(0, 0));
	//this->container_Bar->show();


	//this->array_Bar_Plot1->clear();
	//for (int i = 0; i <= 100; i++) {
	//	QBarDataRow* tempDataRow = new QBarDataRow;
	//	for (int j = 0; j <= 100; j++) {
	//		tempDataRow->append(QBarDataItem(i*j));
	//	}
	//	this->array_Bar_Plot1->append(tempDataRow);
	//}
	//this->proxy_Bar_Plot1->resetArray(this->array_Bar_Plot1);

	// Preparing 3D Surfaces
	this->array_Surface_Plot1 = new QSurfaceDataArray;
	this->proxy_Surface_Plot1 = new QSurfaceDataProxy;
	this->proxy_Surface_Plot1->resetArray(this->array_Surface_Plot1);
	this->series_Surface_Plot1 = new QSurface3DSeries(this->proxy_Surface_Plot1);
	this->series_Surface_Plot1->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
	this->series_Surface_Plot1->setDrawMode(QSurface3DSeries::DrawSurface);
	this->surface_Plot1 = new Q3DSurface();
	this->surface_Plot1->addSeries(this->series_Surface_Plot1);
	this->container_Surface = QWidget::createWindowContainer(this->surface_Plot1);
	this->surface_Plot1->axisX()->setAutoAdjustRange(true);
	this->surface_Plot1->axisY()->setAutoAdjustRange(true);
	this->surface_Plot1->axisZ()->setAutoAdjustRange(true);
	this->surface_Plot1->setFlags(this->scatter_Plot1->flags() ^ Qt::FramelessWindowHint);
	this->surface_Plot1->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
	this->surface_Plot1->axisX()->setTitle("X / mm");
	this->surface_Plot1->axisY()->setTitle("Y / mm");
	this->surface_Plot1->axisZ()->setTitle("Measurement");
	this->surface_Plot1->activeTheme()->setType(Q3DTheme::ThemeDigia);

	this->array_Surface_Plot2 = new QSurfaceDataArray;
	this->proxy_Surface_Plot2 = new QSurfaceDataProxy;
	this->proxy_Surface_Plot2->resetArray(this->array_Surface_Plot2);
	this->series_Surface_Plot2 = new QSurface3DSeries(this->proxy_Surface_Plot2);
	this->series_Surface_Plot2->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
	this->series_Surface_Plot2->setDrawMode(QSurface3DSeries::DrawSurface);
	this->surface_Plot2 = new Q3DSurface();
	this->surface_Plot2->addSeries(this->series_Surface_Plot2);
	this->container_Surface2 = QWidget::createWindowContainer(this->surface_Plot2);
	this->surface_Plot2->axisX()->setAutoAdjustRange(true);
	this->surface_Plot2->axisY()->setAutoAdjustRange(true);
	this->surface_Plot2->axisZ()->setAutoAdjustRange(true);
	this->surface_Plot2->setFlags(this->scatter_Plot1->flags() ^ Qt::FramelessWindowHint);
	this->surface_Plot2->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
	this->surface_Plot2->axisX()->setTitle("X / mm");
	this->surface_Plot2->axisY()->setTitle("Y / mm");
	this->surface_Plot2->axisZ()->setTitle("Measurement");
	this->surface_Plot2->activeTheme()->setType(Q3DTheme::ThemeIsabelle);

}

pasc::~pasc() {
	if (ed_serialPort->isOpen()) {
		ed_serialPort->close();
	}
	if (lc_serialPort->isOpen()) {
		lc_serialPort->close();
	}
	if (em_serialPort->isOpen()) {
		em_serialPort->close();
	}
	if (ft_serialPort->isOpen()) {
		ft_serialPort->close();
	}
	delete ed_serialPort;
	delete lc_serialPort;
	delete em_serialPort;
	delete ft_serialPort;
	//delete ui;
}

void pasc::initUI() {
	this->setWindowTitle("Ender Hack");
	this->refreshPortNameList();
	//ui.comboBox_SensorType->addItems(QStringList({ "Load Cell", "Electrometer"}));
	ui.comboBox_SendTarget->addItems(QStringList({ "Ender", "Sensor" }));
	ui.comboBox_SendTarget->setCurrentIndex(0);
}

QStringList pasc::getPortNameList() {
	QStringList ed_serialPortName;
	QList curAvailablePorts = QSerialPortInfo::availablePorts();
	QSerialPortInfo info;
	for (int portCount = 0; portCount < curAvailablePorts.count(); ++portCount) {
		info = curAvailablePorts.at(portCount);
		ed_serialPortName << info.portName();
	}
	//for each (const QSerialPortInfo & info in curAvailablePorts) {
	//	ed_serialPortName << info.portName();
	//	//qDebug() << "serialPortName:" << info.portName();
	//}
	return ed_serialPortName;
}

void pasc::refreshPortNameList() {
	ui.comboBox_PortNames_Ender->clear();
	ui.comboBox_PortNames_LoadCell->clear();
	ui.comboBox_PortNames_Electrometer->clear();
	ui.comboBox_PortNames_Futek->clear();

	ed_portNameList = getPortNameList();

	ui.comboBox_PortNames_Ender->addItems(ed_portNameList);
	ui.comboBox_PortNames_LoadCell->addItems(ed_portNameList);
	ui.comboBox_PortNames_Electrometer->addItems(ed_portNameList);
	ui.comboBox_PortNames_Futek->addItems(ed_portNameList);
}

void pasc::openPort() {
	//// Close any open port... ////
	if (ed_serialPort->isOpen()) {
		ed_serialPort->clear();
		ed_serialPort->close();
	}
	//// Open the selected port... ////
	ed_serialPort->setPortName(ui.comboBox_PortNames_Ender->currentText());
	if (!ed_serialPort->open(QIODevice::ReadWrite)) {
		qDebug() << "Port Connection Failed...";
		return;
	}
	qDebug() << "Port Connected...";
	//// Port Configuration ////
	ed_serialPort->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
	ed_serialPort->setDataBits(QSerialPort::Data8);
	ed_serialPort->setFlowControl(QSerialPort::NoFlowControl);
	ed_serialPort->setParity(QSerialPort::NoParity);
	ed_serialPort->setStopBits(QSerialPort::OneStop);
	//// ReadyRead Signal ???? ////
	// QObject::connect(ed_serialPort, SIGNAL(readyRead()), this, SLOT(reveiveInfo()));

	QString initMessage("ED: INITIALIZE");
	//QByteArray initEndCode("size: \n585");
	this->sendMessage(initMessage, 0);
}

void pasc::openPortSensor() {
	//// Close any open port... ////
	if (lc_serialPort->isOpen()) {
		lc_serialPort->clear();
		lc_serialPort->close();
	}
	//// Open the selected port... ////
	lc_serialPort->setPortName(ui.comboBox_PortNames_LoadCell->currentText());
	if (!lc_serialPort->open(QIODevice::ReadWrite)) {
		qDebug() << "Sensor Port Connection Failed...";
		return;
	}
	qDebug() << "Sensor Port Connected...";
	//// Port Configuration ////
	lc_serialPort->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections);
	lc_serialPort->setDataBits(QSerialPort::Data8);
	lc_serialPort->setFlowControl(QSerialPort::NoFlowControl);
	lc_serialPort->setParity(QSerialPort::NoParity);
	lc_serialPort->setStopBits(QSerialPort::OneStop);
	//// ReadyRead Signal ???? ////
	// QObject::connect(ed_serialPort, SIGNAL(readyRead()), this, SLOT(reveiveInfo()));

	//qbytearray initmessage("");
	//qbytearray initendcode("size: \n585");
	//this->sendmessage(initmessage, initendcode);
}

void pasc::openPortElectrometer() {
	//// Close any open port... ////
	if (em_serialPort->isOpen()) {
		em_serialPort->clear();
		em_serialPort->close();
	}
	//// Open the selected port... ////
	em_serialPort->setPortName(ui.comboBox_PortNames_Electrometer->currentText());
	if (!em_serialPort->open(QIODevice::ReadWrite)) {
		qDebug() << "Electrometer Port Connection Failed...";
		return;
	}
	qDebug() << "Electrometer Port Connected...";
	//// Port Configuration ////
	em_serialPort->setBaudRate(QSerialPort::Baud57600, QSerialPort::AllDirections);
	em_serialPort->setDataBits(QSerialPort::Data8);
	em_serialPort->setFlowControl(QSerialPort::NoFlowControl);
	em_serialPort->setParity(QSerialPort::NoParity);
	em_serialPort->setStopBits(QSerialPort::OneStop);
	//// ReadyRead Signal ???? ////
	// QObject::connect(ed_serialPort, SIGNAL(readyRead()), this, SLOT(reveiveInfo()));

	//qbytearray initmessage("");
	//qbytearray initendcode("size: \n585");
	//this->sendmessage(initmessage, initendcode);
}

void pasc::openPortFutek() {
	if (ft_serialPort->isOpen()) {
		ft_serialPort->clear();
		ft_serialPort->close();
	}
	//// Open the selected port... ////
	ft_serialPort->setPortName(ui.comboBox_PortNames_Futek->currentText());
	if (!ft_serialPort->open(QIODevice::ReadWrite)) {
		qDebug() << "Futek Port Connection Failed...";
		return;
	}
	qDebug() << "Futek Port Connected...";
	//// Port Configuration ////
	ft_serialPort->setBaudRate(2000000, QSerialPort::AllDirections); // ftdi non-standard baud rate
	ft_serialPort->setDataBits(QSerialPort::Data8);
	ft_serialPort->setFlowControl(QSerialPort::NoFlowControl);
	ft_serialPort->setParity(QSerialPort::NoParity);
	ft_serialPort->setStopBits(QSerialPort::OneStop);
}

void pasc::sendMessage(QString message, bool inLoop) {
	// !! remember to fix initial
	ui.textBrowser_SendHistory->append(QString::number(this->curCodeIndex) + ")->" + message);

	if (message.startsWith("PC: ")) {
		message.remove("PC: ");
		this->pcResponse(message);
		ui.textBrowser_ReadHistory->append(QString::number(this->curCodeIndex) + ")<-" + pc_currentFeedback);
	}
	if (message.startsWith("ED: ")) {
		message.remove("ED: ");
		if (ed_serialPort->isOpen()) {
			this->edResponse(message);
		}
		else
		{
			ed_currentFeedback = "";
		}
		ui.textBrowser_ReadHistory->append(QString::number(this->curCodeIndex) + ")<-" + ed_currentFeedback);
	}
	if (message.startsWith("LC: ")) {
		message.remove("LC: ");
		if (lc_serialPort->isOpen()) {
			this->lcResponse(message);
		}
		else {
			lc_currentFeedback = "";
		}
		ui.textBrowser_ReadHistory->append(QString::number(this->curCodeIndex) + ")<-" + lc_currentFeedback);
	}
	if (message.startsWith("EM: ")) {
		message.remove("EM: ");
		this->emResponse(message);
		ui.textBrowser_ReadHistory->append(QString::number(this->curCodeIndex) + ")<-" + em_currentFeedback);
	}
	if (message.startsWith("FT: ")) {
		message.remove("FT: ");
		this->ftResponse(message);
		ui.textBrowser_ReadHistory->append(QString::number(this->curCodeIndex) + ")<-" + ft_currentFeedback);
	}

	if (inLoop) this->curCodeIndex++;
	// Loop !!
	if (inLoop && this->curCodeIndex < ed_currentGCode.length())	this->timer_Read1 = startTimer(2);
	//if (inLoop && this->curCodeIndex < ed_currentGCode.length() && !message.startsWith("G0"))	this->timer_Read1 = startTimer(2);
}

void pasc::pcResponse(QString message) {

	qDebug() << "PC Responding";

	if (message.contains("AcquireProfile")) {
		//double curTimeReading = double(this->timeStampTimer.elapsed()) / 1000;
		curTimeReading = double(this->timeStampTimer.elapsed()) / 1000;

		// Get Position Reading
		//QVector<int> tempIndices(4);
		QVector<long long> tempIndices(4);
		if (ed_serialPort->isOpen()) {
			tempIndices = { ed_currentFeedback.indexOf("X:"), ed_currentFeedback.indexOf("Y:"), ed_currentFeedback.indexOf("Z:"), ed_currentFeedback.indexOf("E:"),ed_currentFeedback.indexOf("Count") };
			for (int i = 0; i < 4; i++) {
				this->curXYZE[i] = ed_currentFeedback.mid(tempIndices[i] + 2, tempIndices[i + 1] - tempIndices[i] - 3).toDouble();
			}
		}
		else {
			for (int i = 0; i < 4; i++) {
				this->curXYZE[i] = 0;
			}
		}

		//double curForceReading = 0;
		curForceReading = 0;
		if (lc_serialPort->isOpen() && ui.checkBox_LinearScanLoadCellEnabled->isChecked()) {
			QString lc_currentFeedback_Local = lc_currentFeedback;
			lc_currentFeedback_Local.truncate(lc_currentFeedback.indexOf("\r\n") - 1);
			//this->lc_currentFeedback.truncate(lc_currentFeedback.indexOf("\r\n")-1);
			curForceReading = lc_currentFeedback_Local.toDouble();
		}

		//double curElecReading = 0;
		curElecReading = 0;
		if (em_serialPort->isOpen() && ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
			if (this->em_currentFeedback.contains(",")) {
				this->em_currentFeedback.truncate(em_currentFeedback.indexOf(","));				
			}
			curElecReading = em_currentFeedback.toDouble();
		}

		curFutekReading = 0;
		if (ft_serialPort->isOpen() && ui.checkBox_FutekEnabled->isChecked()) {
			if (this->ft_currentFeedback.contains("lb")) {
				this->ft_currentFeedback.truncate(ft_currentFeedback.indexOf("l"));				
			}
			curFutekReading = ft_currentFeedback.toDouble();
			curFutekReading = curFutekReading * 4.4482189159; // lb to N
		}

		curPressureReading = 0;
		curPressureReading = pow(10,(curElecReading-5));

		// Setting data range
		if (this->data_Time.isEmpty()) {
			this->data_Time_Range[0] = curTimeReading; this->data_Time_Range[1] = curTimeReading;
			this->data_Force_Range[0] = curForceReading; this->data_Force_Range[1] = curForceReading;
			this->data_Elec_Range[0] = curElecReading; this->data_Elec_Range[1] = curElecReading;

			//this->data_X_Range[0] = curXYZE[0]; this->data_X_Range[1] = curXYZE[0];
			this->data_Y_Range[0] = curXYZE[1]; this->data_Y_Range[1] = curXYZE[1];
			this->data_Z_Range[0] = curXYZE[2]; this->data_Z_Range[1] = curXYZE[2];
			this->data_E_Range[0] = curXYZE[3]; this->data_E_Range[1] = curXYZE[3];

			this->data_Pressure_Range[0] = curPressureReading; this->data_Pressure_Range[1] = curPressureReading;
			this->data_Futek_Range[0] = curFutekReading; this->data_Futek_Range[1] = curFutekReading;
		}
		if (curTimeReading < this->data_Time_Range[0]) this->data_Time_Range[0] = curTimeReading;
		if (curTimeReading > this->data_Time_Range[1]) this->data_Time_Range[1] = curTimeReading;
		if (curForceReading < this->data_Force_Range[0]) this->data_Force_Range[0] = curForceReading;
		if (curForceReading > this->data_Force_Range[1]) this->data_Force_Range[1] = curForceReading;
		if (curElecReading < this->data_Elec_Range[0]) this->data_Elec_Range[0] = curElecReading;
		if (curElecReading > this->data_Elec_Range[1]) this->data_Elec_Range[1] = curElecReading;
		if (curXYZE[1] < this->data_Y_Range[0]) this->data_Y_Range[0] = curXYZE[1];
		if (curXYZE[1] > this->data_Y_Range[1]) this->data_Y_Range[1] = curXYZE[1];
		if (curXYZE[2] < this->data_Z_Range[0]) this->data_Z_Range[0] = curXYZE[2];
		if (curXYZE[2] > this->data_Z_Range[1]) this->data_Z_Range[1] = curXYZE[2];

		if (curFutekReading < this->data_Futek_Range[0]) this->data_Futek_Range[0] = curFutekReading;
		if (curFutekReading > this->data_Futek_Range[1]) this->data_Futek_Range[1] = curFutekReading;

		if (curPressureReading < this->data_Pressure_Range[0]) this->data_Pressure_Range[0] = curPressureReading;
		if (curPressureReading > this->data_Pressure_Range[1]) this->data_Pressure_Range[1] = curPressureReading;

		this->data_Time.append(curTimeReading);
		this->data_Force.append(curForceReading);
		this->data_Elec.append(curElecReading);
		this->data_X.append(curXYZE[0]);
		this->data_Y.append(curXYZE[1]);
		this->data_Z.append(curXYZE[2]);
		this->data_Futek.append(curFutekReading);
		this->data_Pressure.append(curPressureReading);

		//this->array_Scatter_Plot1->append(QScatterDataItem(QVector3D(this->curXYZE[0], this->curXYZE[1], curForceReading)));

		// Debug output
		ui.label_debugReading_1->setText("Futek: " + QString::number(curFutekReading - ui.lineEdit_FutekZero->text().toDouble()) + " / N");
		ui.label_debugReading_2->setText("Stinger: " + QString::number(curPressureReading) + " / Torr");
		ui.label_debugReading_3->setText("Mark10: " + QString::number(curForceReading) + " / N");
		ui.label_debugReading_4->setText("Ender Z: " + QString::number(curXYZE[2]) + " / mm");


		// Data Visualization for LinearScan
		if (this->curProfileIndex == 1) {
			// Updating 2D Plot
			this->series_Plot1->append(this->data_Time.last(), this->data_Force.last());
			this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
			this->chart_Plot1->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);

			// Updating 3d Plot
			double xStep = ui.lineEdit_XLimLinearScan->text().toDouble() / ui.lineEdit_XSegLinearScan->text().toInt();
			double yStep = ui.lineEdit_YLimLinearScan->text().toDouble() / ui.lineEdit_YSegLinearScan->text().toInt();
			if (ui.checkBox_SurfacePlot->isChecked()) {
				//qDebug() << int(round(curXYZE[1] / yStep)) << "," << int(round(curXYZE[0] / yStep));
				//qDebug() << this->proxy_Surface_Plot1->rowCount() << "," << this->proxy_Surface_Plot1->columnCount();
				if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) this->proxy_Surface_Plot1->setItem(int(round(curXYZE[1] / yStep)), int(round(this->curXYZE[0] / xStep)), QSurfaceDataItem(QVector3D(curXYZE[0], -curForceReading * 1000, curXYZE[1])));
				if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) this->proxy_Surface_Plot2->setItem(int(round(curXYZE[1] / yStep)), int(round(this->curXYZE[0] / xStep)), QSurfaceDataItem(QVector3D(curXYZE[0], curElecReading, curXYZE[1])));
			}
			if (ui.checkBox_BarPlot->isChecked()) {
				this->proxy_Bar_Plot1->setItem(int(round(curXYZE[1] / yStep)), int(round(this->curXYZE[0] / xStep)), QBarDataItem(curForceReading < 0 ? -curForceReading * 1000 : 0));
			}
			if (ui.checkBox_ScatterPlot->isChecked()) {
				this->proxy_Scatter_Plot1->addItem(QScatterDataItem(QVector3D(this->curXYZE[0], curForceReading, this->curXYZE[1])));
			}
		}

		// Data Visualization for Compression
		if (this->curProfileIndex == 2) {
			this->series_Plot1->append(this->data_Time.last(), this->data_Force.last());
			this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
			this->chart_Plot1->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);

			this->series_Plot2->append(this->data_Time.last(), this->data_Elec.last());
			this->chart_Plot2->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
			this->chart_Plot2->axisY()->setRange(data_Elec_Range[0], data_Elec_Range[1]);
		}

		// Data Visualization for TimeElapse
		if (this->curProfileIndex == 3) {
			//Limiting Samples
			double sampleTime = ui.lineEdit_TimeElapseSampleTime->text().toDouble();
			if (this->data_Time.length() > 1) {
				double deltaTime = this->data_Time.at(data_Time.length() - 1) - this->data_Time.at(data_Time.length() - 2);
				if (deltaTime < sampleTime) {
					this->data_Time.remove(data_Time.length() - 1);
					this->data_Force.remove(data_Force.length() - 1);
					this->data_Elec.remove(data_Elec.length() - 1);
					this->data_X.remove(data_X.length() - 1);
					this->data_Y.remove(data_Y.length() - 1);
					this->data_Z.remove(data_Z.length() - 1);
					this->data_Futek.remove(data_Futek.length() - 1);
					this->data_Pressure.remove(data_Pressure.length() - 1);
				}
				else {
					// Updating 2D Plot
					/*this->series_Plot1->append(this->data_Time.last(), this->data_Force.last());
					this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot1->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);*/

					this->series_Plot1->append(this->data_Time.last(), this->data_Futek.last());
					this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot1->axisY()->setRange(data_Futek_Range[0], data_Futek_Range[1]);
					this->chart_Plot1->setTitle("Futek LC F / N");

					this->series_Plot2->append(this->data_Time.last(), this->data_Elec.last());
					this->chart_Plot2->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot2->axisY()->setRange(data_Elec_Range[0], data_Elec_Range[1]);

					ui.textBrowser_ReadHistory->clear();
					ui.textBrowser_SendHistory->clear();
				}
			}
		}

		// Data Visualization for ChargeCollect
		if (this->curProfileIndex == 4) {
			//Limiting Samples
			double sampleTime = ui.lineEdit_TimeElapseSampleTime->text().toDouble();

			// Updating 2D Plot
			this->series_Plot1->append(this->data_Y.last(), this->data_Elec.last() * 1E9);
			this->chart_Plot1->axisX()->setRange(data_Y_Range[0], data_Y_Range[1]);
			this->chart_Plot1->axisY()->setRange(data_Elec_Range[0] * 1E9, data_Elec_Range[1] * 1E9);

			this->series_Plot2->append(this->data_Time.last(), this->data_Elec.last() * 1E9);
			this->chart_Plot2->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
			this->chart_Plot2->axisY()->setRange(data_Elec_Range[0] * 1E9, data_Elec_Range[1] * 1E9);

			ui.textBrowser_ReadHistory->clear();
			ui.textBrowser_SendHistory->clear();
		}

		// Data Visualization for ZProbe
		if (this->curProfileIndex == 5) {
			// Updating 2D Plot
			this->series_Plot1->append(this->data_Z.last(), this->data_Elec.last() * 1E9);
			this->chart_Plot1->axisX()->setRange(data_Z_Range[0], data_Z_Range[1]);
			this->chart_Plot1->axisY()->setRange(data_Elec_Range[0] * 1E9, data_Elec_Range[1] * 1E9);

			this->series_Plot2->append(this->data_Z.last(), this->data_Force.last());
			this->chart_Plot2->axisX()->setRange(data_Z_Range[0], data_Z_Range[1]);
			this->chart_Plot2->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);

			ui.textBrowser_ReadHistory->clear();
			ui.textBrowser_SendHistory->clear();
		}

		// Data Visualization for FormalCT
		if (this->curProfileIndex == 6) {
			// Updating 2D Plot
			double sampleTime = ui.lineEdit_FormalCT_TimeSample->text().toDouble();
			if (this->data_Time.length() > 1) {
				double deltaTime = this->data_Time.at(data_Time.length() - 1) - this->data_Time.at(data_Time.length() - 2);
				if (deltaTime < sampleTime && sampleTimeOn) {
					this->data_Time.remove(data_Time.length() - 1);
					this->data_Force.remove(data_Force.length() - 1);
					this->data_Elec.remove(data_Elec.length() - 1);
					this->data_X.remove(data_X.length() - 1);
					this->data_Y.remove(data_Y.length() - 1);
					this->data_Z.remove(data_Z.length() - 1);
				}
				else {
					// Updating 2D Plot
					this->series_Plot1->append(this->data_Time.last(), this->data_Elec.last() * 1E9);
					this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot1->axisY()->setRange(data_Elec_Range[0] * 1E9, data_Elec_Range[1] * 1E9);

					this->series_Plot2->append(this->data_Time.last(), this->data_Force.last());
					this->chart_Plot2->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot2->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);

					ui.textBrowser_ReadHistory->clear();
					ui.textBrowser_SendHistory->clear();
				}
			}
		}

		// Data Visualization for Kirigami
		if (this->curProfileIndex == 7) {
			// Updating 2D Plot
			this->series_Plot1->append(this->data_Time.last(), this->data_Force.last());
			this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
			this->chart_Plot1->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);

			this->series_Plot2->append(this->data_Z.last(), -this->data_Force.last());
			this->chart_Plot2->axisX()->setRange(data_Z_Range[0], data_Z_Range[1]);
			this->chart_Plot2->axisY()->setRange(-data_Force_Range[1], -data_Force_Range[0]);

			ui.textBrowser_ReadHistory->clear();
			ui.textBrowser_SendHistory->clear();
		}

		// Data Visualization for Contact Futek
		if (this->curProfileIndex == 8) {
			double sampleTime = ui.lineEdit_CTFutek_SampleTime->text().toDouble();
			if (this->data_Time.length() > 1) {
				double deltaTime = this->data_Time.at(data_Time.length() - 1) - this->data_Time.at(data_Time.length() - 2);
				if (deltaTime < sampleTime && sampleTimeOn) {
					this->data_Time.remove(data_Time.length() - 1);
					this->data_Force.remove(data_Force.length() - 1);
					this->data_Elec.remove(data_Elec.length() - 1);
					this->data_X.remove(data_X.length() - 1);
					this->data_Y.remove(data_Y.length() - 1);
					this->data_Z.remove(data_Z.length() - 1);
					this->data_Futek.remove(data_Futek.length() - 1);
					this->data_Pressure.remove(data_Pressure.length() - 1);
				}
				else {
					// Updating 2D Plot
					/*this->series_Plot1->append(this->data_Time.last(), this->data_Elec.last() * 1E9);
					this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot1->axisY()->setRange(data_Elec_Range[0] * 1E9, data_Elec_Range[1] * 1E9);*/

					this->series_Plot1->append(this->data_Time.last(), this->data_Futek.last());
					this->chart_Plot1->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot1->axisY()->setRange(data_Futek_Range[0], data_Futek_Range[1]);

					this->series_Plot2->append(this->data_Time.last(), this->data_Force.last());
					this->chart_Plot2->axisX()->setRange(data_Time_Range[0], data_Time_Range[1]);
					this->chart_Plot2->axisY()->setRange(data_Force_Range[0], data_Force_Range[1]);

					ui.textBrowser_ReadHistory->clear();
					ui.textBrowser_SendHistory->clear();
				}
			}
		}

		ui.textBrowser_ReadHistory->append(QString::number(this->data_Time.last()));
		pc_currentFeedback = "PC: Data Acquired - Profile";
	}

	if (message.contains("SampleTimeOn")) {
		sampleTimeOn = true;
		pc_currentFeedback = "PC: Sample Time turned on";
	}

	if (message.contains("SampleTimeOff")) {
		sampleTimeOn = false;
		pc_currentFeedback = "PC: Sample Time turned off";
	}

	if (message.contains("AcquireSingle")) {
		//ed_currentFeedback.truncate(ed_currentFeedback.indexOf("Count"));
		//QVector<double> curXYZE(4);
		//QVector<int> tempIndices(4);
		QVector<long long> tempIndices(4);
		qDebug() << "Acquiring Single ... 0";
		tempIndices = { ed_currentFeedback.indexOf("X:"), ed_currentFeedback.indexOf("Y:"), ed_currentFeedback.indexOf("Z:"), ed_currentFeedback.indexOf("E:"),ed_currentFeedback.indexOf("Count") };
		qDebug() << "Acquiring Single ... 1";
		for (int i = 0; i < 4; i++) {
			this->curXYZE[i] = ed_currentFeedback.mid(tempIndices[i] + 2, tempIndices[i + 1] - tempIndices[i] - 3).toDouble();
		}
		ui.lineEdit_XSet->setText(QString::number(this->curXYZE[0]));
		ui.lineEdit_YSet->setText(QString::number(this->curXYZE[1]));
		ui.lineEdit_ZSet->setText(QString::number(this->curXYZE[2]));
		ui.lineEdit_ESet->setText(QString::number(this->curXYZE[3]));

		pc_currentFeedback = "PC: Data Acquired - Single";


	}

	if (message.contains("DELAYMILISECONDS ")) {
		message.remove("DELAYMILISECONDS ");
		QTime dieTime = QTime::currentTime().addMSecs(message.toInt());
		while (QTime::currentTime() < dieTime) {
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
		}
		pc_currentFeedback = "PC: Paused";
	}

	if (message.contains("LOOP ")) {
		qDebug() << "looping";
		message.remove("LOOP ");
		if (message.contains("UNTIL")) {
			qDebug() << "loopingUNTIL";
			QString ms = message;
			ms.truncate(ms.indexOf(" UNTIL"));
			int loopNumber = ms.toInt();
			message = message.right(message.length() - message.indexOf("UNTIL "));
			message.remove("UNTIL ");
			if (message.startsWith("FGREATER ")) {
				qDebug() << "COMPARING F";
				message.remove("FGREATER ");
				//double curForceReading = 999;
				curForceReading = 999;
				double FLimit = message.toDouble();


				if (lc_serialPort->isOpen() && ui.checkBox_LinearScanLoadCellEnabled->isChecked()) {
					QString lc_currentFeedback_Local = lc_currentFeedback;
					lc_currentFeedback_Local.truncate(lc_currentFeedback.indexOf("\r\n") - 1);
					//this->lc_currentFeedback.truncate(lc_currentFeedback.indexOf("\r\n")-1);
					curForceReading = lc_currentFeedback_Local.toDouble();
				}
				//if (lc_serialPort->isOpen() && ui.checkBox_LinearScanLoadCellEnabled->isChecked()) {
				//	this->lc_currentFeedback.truncate(lc_currentFeedback.indexOf("\r\n") - 1);
				//	curForceReading = lc_currentFeedback.toDouble();
				//}

				qDebug() << curForceReading;
				qDebug() << FLimit;
				if (abs(curForceReading) < FLimit) {
					this->curCodeIndex = this->curCodeIndex - loopNumber - 1;
					qDebug() << "backSpace" << curCodeIndex;
				}

			}

			if (message.startsWith("ZGREATER ")) {
				qDebug() << "COMPARING Z";
				message.remove("ZGREATER ");
				//double curForceReading = 999;
				double curZReading = curXYZE[2];
				double ZLimit = message.toDouble();
				qDebug() << curZReading;
				qDebug() << ZLimit;
				if (curZReading < ZLimit) {
					this->curCodeIndex = this->curCodeIndex - loopNumber - 1;
					qDebug() << "backSpace" << curCodeIndex;
				}

			}

			if (message.startsWith("TGREATER ")) {
				qDebug() << "COMPARING F";
				message.remove("TGREATER ");

				if (!isLoopTiming) {
					double TLimit = message.toDouble();
					loopTimingLimit = curTimeReading + TLimit;
					isLoopTiming = true;
				}
				if (abs(curTimeReading) < loopTimingLimit) {
					this->curCodeIndex = this->curCodeIndex - loopNumber - 1;
					qDebug() << "backSpace" << curCodeIndex;
					qDebug() << "curTime" << curTimeReading;
					qDebug() << "targetTime" << loopTimingLimit;
				}
				else {
					isLoopTiming = false;
				}

			}
		}

	}

}

void pasc::edResponse(QString message) {
	qDebug() << "Ender Inquiring";
	QByteArray messageBA;
	if (message.contains("INITIALIZE")) {
		message.remove("INITIALIZE");
		ed_CurEndCode = "size: \n585";
	}
	else
	{
		ed_CurEndCode = "ok";
	}
	message.append("\n");
	messageBA = message.toUtf8();
	//messageBA.append("\n");
	if (!messageBA.isEmpty() && !messageBA.isNull()) {
		ed_serialPort->write(messageBA);
	}
	//qDebug() << "sent >>" << messageBA;
	//ui.textBrowser_SendHistory->append(QString(messageBA));
	//ui.textBrowser_SendHistory->repaint();
	//ed_serialPort->waitForReadyRead(-1);
	qDebug() << "Ender Responding";
	QByteArray feedbackMessage = this->readMessage();
	while (feedbackMessage.isEmpty() || !(feedbackMessage.contains(ed_CurEndCode))) {
		qDebug() << "Ender Streaming... 0";
		//ed_serialPort->waitForReadyRead(100);
		qDebug() << "Ender Streaming... 0.1";
		if (!this->container_Surface->isHidden()) {
			this->container_Surface->update();
			//this->surface_Plot1->requestUpdate();
		}
		if (!this->container_Surface2->isHidden()) this->container_Surface2->update();
		qDebug() << "Ender Streaming... 0.2";
		QCoreApplication::processEvents();
		qDebug() << "Ender Streaming... 1";
		feedbackMessage.append(this->readMessage());
		//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
	}
	//ui.textBrowser_ReadHistory->append(QString(feedbackMessage));
	//this->update();
	ed_currentFeedback = QString(feedbackMessage);
	qDebug() << "Ender Responded";
	//if (message.startsWith("G0")) {
	//	ed_currentMessage = message;
	//}
	//else {
	//	QByteArray feedbackMessage = this->readMessage();
	//	while (feedbackMessage.isEmpty() || !(feedbackMessage.contains(ed_CurEndCode))) {
	//		ed_serialPort->waitForReadyRead(1);
	//		feedbackMessage.append(this->readMessage());
	//		//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
	//	}
	//	//ui.textBrowser_ReadHistory->append(QString(feedbackMessage));
	//	//this->update();
	//	ed_currentFeedback = QString(feedbackMessage);
	//}

	//this->timer_ED1 = startTimer(2);
}

void pasc::readEDduringTravel() {
	if (ed_currentMessage.startsWith("G0")) {
		ed_CurEndCode = "ok";
		QByteArray feedbackMessage = this->readMessage();
		while (feedbackMessage.isEmpty() || !(feedbackMessage.contains(ed_CurEndCode))) {
			//ed_serialPort->waitForReadyRead(100); // The Qlist Bug

			//QCoreApplication::processEvents();

			feedbackMessage.append(this->readMessage());
			//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
		}
		//ui.textBrowser_ReadHistory->append(QString(feedbackMessage));
		//this->update();
		ed_currentFeedback = QString(feedbackMessage);
		this->timer_Read1 = startTimer(2);
	}

}

void pasc::lcResponse(QString message) {
	QByteArray endCode = "\r\n";
	QByteArray messageBA;
	message.append("\r\n");
	messageBA = message.toUtf8();
	//lc_serialPort->write(endCode);
	//QByteArray feedbackMessage = lc_serialPort->readLine();
	//messageBA.append("\r\n");
	lc_serialPort->write(messageBA);
	QCoreApplication::processEvents(); // Important!! Otherwise data will always have 1 cycle lag!!
	int i = 1;
	QByteArray feedbackMessage = lc_serialPort->readLine();
	//lc_serialPort->write("");
	while (feedbackMessage.isEmpty() || !feedbackMessage.contains(endCode)) {
		//lc_serialPort->waitForReadyRead(200); // The Qlist Bug
		feedbackMessage.append(lc_serialPort->readLine());
		//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
		i++;
		if (i > 1000) break;
	}
	qDebug() << "received >> " << feedbackMessage;
	lc_currentFeedback = QString(feedbackMessage);

	//this->timer_LC1 = startTimer(2);
}

void pasc::emResponse(QString message) {
	qDebug() << "Inquiring Keithley";
	QByteArray endCode = "\r";
	QByteArray messageBA;
	message.append("\r");
	messageBA = message.toUtf8();
	//lc_serialPort->write(endCode);
	//QByteArray feedbackMessage = lc_serialPort->readLine();
	//messageBA.append("\r\n");

	em_serialPort->write(messageBA);
	em_serialPort->waitForBytesWritten(2000);
	QCoreApplication::processEvents(); // Important!! Otherwise data will always have 1 cycle lag!!
	qDebug() << "Keithley Responding";
	int i = 1;
	//QByteArray feedbackMessage = em_serialPort->readAll();
	QByteArray feedbackMessage = em_serialPort->readLine();
	while (feedbackMessage.isEmpty() || !feedbackMessage.contains(endCode)) {
		//QCoreApplication::processEvents();
		if (messageBA.contains("?")) em_serialPort->waitForReadyRead(2000); 
		feedbackMessage.append(em_serialPort->readLine());
		//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
		i++;
		qDebug() << i;
		if (i > 3000) break;
	}
	qDebug() << "received >> " << feedbackMessage;
	em_currentFeedback = QString(feedbackMessage);
}

QByteArray pasc::readMessage() {
	QByteArray message = ed_serialPort->readLine();
	qDebug() << "received >> " << message;
	return message;
}

void pasc::ftResponse(QString message) {
	QCoreApplication::processEvents();
	QByteArray feedbackMessage = ft_serialPort->readAll();
	if (feedbackMessage.contains("l")) {
		feedbackMessage.truncate(feedbackMessage.lastIndexOf("l"));
		if (feedbackMessage.contains("l")) {
			feedbackMessage = feedbackMessage.last(feedbackMessage.size() - feedbackMessage.lastIndexOf("l") -3 );
			//if (feedbackMessage.startsWith("\n")) {
			//	feedbackMessage.erase()
			//}
		}
	}
	qDebug() << "debug >> " << QString::number(feedbackMessage.size() - feedbackMessage.lastIndexOf("l"));
	//QByteArray feedbackMessage = "";
	//do {
	//	feedbackMessage = ft_serialPort->readLine();
	//} while (feedbackMessage == "");
	//qDebug() << "received >> " << feedbackMessage;
	ft_currentFeedback = QString(feedbackMessage);

	if (ft_currentFeedback.contains("\n")) {
		ft_currentFeedback.remove("\n");
	}
	qDebug() << "received >> " << ft_currentFeedback;
}

void pasc::sendFromLineEdit() {
	//// Send message from line edit field... ////
	QByteArray endCode("ok");
	QString singleMessage("");
	singleMessage = ui.lineEdit_SendMessage->text();
	//singleMessage = this->serialPrefixes.at(ui.comboBox_SendTarget->currentIndex()) + singleMessage;
	//switch (ui.comboBox_SendTarget->currentIndex()) {
	//case 0: singleMessage = this->serialPrefixes.at(0) + singleMessage;
	//	break;
	//case 1: singleMessage = this->serialPrefixes.at(1) + singleMessage;
	//	break;
	//}
	ui.lineEdit_SendMessage->setText(singleMessage);
	this->sendMessage(singleMessage, 0);

	//sendMessage(ui.lineEdit_SendMessage->text().toUtf8(),endCode);
}

void pasc::singleReadXYZE() {
	this->haltCurrentProfile();
	qDebug() << "Acquiring Single ... -1";
	this->sendMessage("ED: M114", 0);
	this->sendMessage("PC: AcquireSingle", 0);
}

void pasc::setCurrentXYZE() {
	sendMessage("ED: G92 X" + ui.lineEdit_XSet->text(), 0);
	sendMessage("ED: G92 Y" + ui.lineEdit_YSet->text(), 0);
	sendMessage("ED: G92 Z" + ui.lineEdit_ZSet->text(), 0);
	sendMessage("ED: G92 E" + ui.lineEdit_ESet->text(), 0);
	this->singleReadXYZE();
}

void pasc::sendCurrentProfile() {
	this->curProfileIndex = ui.comboBox_CurrentProfile->currentIndex();
	this->em_currentFeedback = "";
	this->lc_currentFeedback = "";

	// Preparing Data Acquisition	
	this->profileHalted = false;
	this->data_Force.clear(); this->data_Force_Range.clear();
	this->data_Time.clear();  this->data_Time_Range.clear();
	this->data_X.clear(); this->data_X_Range.clear();
	this->data_Y.clear(); this->data_Y_Range.clear();
	this->data_Z.clear(); this->data_Z_Range.clear();
	this->data_E.clear(); this->data_E_Range.clear();
	this->data_Elec.clear(); this->data_Elec_Range.clear();
	this->data_Futek.clear(); this->data_Futek_Range.clear();
	this->data_Pressure.clear(); this->data_Pressure_Range.clear();


	this->data_Force_Range.append(0); this->data_Force_Range.append(0);
	this->data_Time_Range.append(0);  this->data_Time_Range.append(0);
	this->data_Elec_Range.append(0); this->data_Elec_Range.append(0);
	this->data_X_Range.append(0); this->data_X_Range.append(0);
	this->data_Y_Range.append(0); this->data_Y_Range.append(0);
	this->data_Z_Range.append(0); this->data_Z_Range.append(0);
	this->data_E_Range.append(0); this->data_E_Range.append(0);
	this->data_Futek_Range.append(0); this->data_Futek_Range.append(0);
	this->data_Pressure_Range.append(0); this->data_Pressure_Range.append(0);


	this->timeStampTimer.start();
	this->series_Plot1->clear();
	this->series_Plot2->clear();

	// Preparing 2D Data Visualization
	//this->series_Plot1 = new QLineSeries();
	//this->chart_Plot1 = new QChart();
	//this->chart_Plot1->legend()->hide();
	//this->chart_Plot1->addSeries(this->series_Plot1);
	//this->chart_Plot1->createDefaultAxes();
	//this->chart_Plot1->setTitle("Test");

	this->proxy_Scatter_Plot1->removeItems(0, this->proxy_Scatter_Plot1->itemCount());
	//ui.graphicsView_Debug->setChart(this->chart_Plot1);
	//ui.graphicsView_Debug->setRenderHint(QPainter::Antialiasing);

	this->container_Scatter->hide();
	this->container_Bar->hide();
	this->container_Surface->hide();
	this->container_Surface2->hide();

	if (this->curProfileIndex == 1) {
		if (ui.checkBox_SurfacePlot->isChecked()) {
			this->container_Surface->show();
			this->container_Surface2->show();
		}
		if (ui.checkBox_BarPlot->isChecked()) {
			this->container_Bar->show();
		}
		if (ui.checkBox_ScatterPlot->isChecked()) {
			this->container_Scatter->show();
		}
	}
	if (this->curProfileIndex == 4) {
		this->chart_Plot1->setTitle("Charge Collected / nC");
		this->chart_Plot2->setTitle("Charge Collected / nC");
	}




	// Start the Loop !!
	curCodeIndex = 0;
	this->timer_Read1 = startTimer(2);
}

void pasc::haltCurrentProfile() {
	this->profileHalted = true;
}

void pasc::reveiveInfo() {
	QByteArray info = ed_serialPort->readAll();
	qDebug() << "received >> " << info;
}

void pasc::composeHeader() {
	const int allHeaderCount = 7;
	bool allHeaderChecks[allHeaderCount] = {
		ui.checkBox_Header_AllowcoldExtrusion->isChecked(),
		ui.checkBox_Header_FanOff->isChecked(),
		ui.checkBox_Header_HomeAxis->isChecked(),
		ui.checkBox_Header_XStep->isChecked(),
		ui.checkBox_Header_YStep->isChecked(),
		ui.checkBox_Header_ZStep->isChecked(),
		ui.checkBox_Header_EStep->isChecked()
	};
	QStringList allHeaderCommands = {
		this->serialPrefixes.at(0) + QString("M302 S0"),
		this->serialPrefixes.at(0) + QString("M107"),
		this->serialPrefixes.at(0) + QString("G28"),
		this->serialPrefixes.at(0) + QString("M92 X") + ui.lineEdit_Header_XStep->text(),
		this->serialPrefixes.at(0) + QString("M92 Y") + ui.lineEdit_Header_YStep->text(),
		this->serialPrefixes.at(0) + QString("M92 Z") + ui.lineEdit_Header_ZStep->text(),
		this->serialPrefixes.at(0) + QString("M92 E") + ui.lineEdit_Header_EStep->text()
	};
	ed_currentGCode.clear();
	for (int tempCount = 0; tempCount < allHeaderCount; tempCount++) {
		if (allHeaderChecks[tempCount] == true) {
			ed_currentGCode << allHeaderCommands.at(tempCount);
		}
	}
	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::composeHeaderVoltage() {
	ed_currentGCode << QString("EM: *rst");
	ed_currentGCode << QString("EM: *CLS");
	ed_currentGCode << QString("EM: FUNC 'VOLT'");
	ed_currentGCode << QString("EM: FORM:ELEM READ,TIME");
	ed_currentGCode << QString("EM: VOLT:RANG:AUTO ON");
	ed_currentGCode << QString("EM: VOLT:GUAR OFF");
	ed_currentGCode << QString("EM: VOLT:NPLC 1");
	ed_currentGCode << QString("EM: SYST:AZER OFF");
	ed_currentGCode << QString("EM: AVER OFF");
	ed_currentGCode << QString("EM: STAT:MEAS:ENAB 512");
	ed_currentGCode << QString("EM: DISP:ENAB ON");
	ed_currentGCode << QString("EM: ARM:COUN 1");
	ed_currentGCode << QString("EM: TRIG:COUN 1");
	ed_currentGCode << QString("EM: INIT");
	ed_currentGCode << QString("EM: SYST:ZCOR:ACQ");
	ed_currentGCode << QString("EM: SYST:ZCH OFF");
	ed_currentGCode << QString("EM: SYST:ZCOR OFF");
}

void pasc::composeHeaderCharge() {
	ed_currentGCode << QString("EM: *rst");
	ed_currentGCode << QString("EM: *CLS");
	ed_currentGCode << QString("EM: FUNC 'CHAR'");
	ed_currentGCode << QString("EM: FORM:ELEM READ,TIME");
	ed_currentGCode << QString("EM: VOLT:RANG:AUTO ON");
	ed_currentGCode << QString("EM: VOLT:GUAR OFF");
	ed_currentGCode << QString("EM: VOLT:NPLC 1");
	ed_currentGCode << QString("EM: SYST:AZER OFF");
	ed_currentGCode << QString("EM: AVER OFF");
	ed_currentGCode << QString("EM: STAT:MEAS:ENAB 512");
	ed_currentGCode << QString("EM: DISP:ENAB ON");
	ed_currentGCode << QString("EM: ARM:COUN 1");
	ed_currentGCode << QString("EM: TRIG:COUN 1");
	ed_currentGCode << QString("EM: INIT");
	ed_currentGCode << QString("EM: SYST:ZCOR:ACQ");
	ed_currentGCode << QString("EM: SYST:ZCH ON");
	ed_currentGCode << QString("EM: SYST:ZCOR OFF");
}

void pasc::composeHeaderChargeReady() {
	ed_currentGCode << QString("EM: SYST:ZCH OFF");
	ed_currentGCode << QString("EM: SYST:ZCOR ON");
	ed_currentGCode << QString("EM: CALC2:NULL:STAT ON");
}

void pasc::composeLinearScan() {
	this->curProfileIndex = 1;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);

	ed_currentGCode.clear();
	int xSeg = ui.lineEdit_XSegLinearScan->text().toInt();
	int ySeg = ui.lineEdit_YSegLinearScan->text().toInt();
	double xLim = ui.lineEdit_XLimLinearScan->text().toDouble();
	double yLim = ui.lineEdit_YLimLinearScan->text().toDouble();
	double xStep = xLim / xSeg;
	double yStep = yLim / ySeg;
	double fSpeed = ui.lineEdit_FLinearScan->text().toDouble();

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		this->composeHeaderVoltage();
	}

	for (int i = 0; i <= ySeg; i++) {
		ed_currentGCode << QString("ED: G0 Y" + QString::number(round(yStep * i * 100) / 100) + " F" + QString::number(round(fSpeed)));

		//if (i > 0) {
			//ed_currentGCode << QString("ED: G4 P200");
		ed_currentGCode << QString("ED: M114");
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		//}
		for (int j = 1; j <= xSeg; j++) {
			double destination = round(xStep * j * 100) / 100;
			if (i % 2 != 0) destination = xLim - destination;
			ed_currentGCode << QString("ED: G0 X" + QString::number(destination) + " F" + QString::number(round(fSpeed)));
			//ed_currentGCode << QString("ED: G4 P200");
			ed_currentGCode << QString("ED: M114");
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
		}
		//ed_currentGCode << QString("ED: G0 Y" + QString::number(round(yStep * i * 100) / 100) + " F" + QString::number(round(fSpeed)));
	}

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		ed_currentGCode << QString("EM: SYST:ZCH ON");
	}

	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));

	//Initializing Particular 3D Surface Plot 
	this->array_Surface_Plot1->clear();
	for (int i = 0; i <= ySeg; i++) {
		QSurfaceDataRow* tempDataRow = new QSurfaceDataRow;
		for (int j = 0; j <= xSeg; j++) {
			tempDataRow->append(QVector3D(j * xStep, 0, i * yStep));
		}
		this->array_Surface_Plot1->append(tempDataRow);
	}
	this->proxy_Surface_Plot1->resetArray(this->array_Surface_Plot1);

	this->array_Surface_Plot2->clear();
	for (int i = 0; i <= ySeg; i++) {
		QSurfaceDataRow* tempDataRow = new QSurfaceDataRow;
		for (int j = 0; j <= xSeg; j++) {
			tempDataRow->append(QVector3D(j * xStep, 0, i * yStep));
		}
		this->array_Surface_Plot2->append(tempDataRow);
	}
	this->proxy_Surface_Plot2->resetArray(this->array_Surface_Plot2);

	// Initiating Particular 3D Bar Plot
	this->array_Bar_Plot1->clear();
	for (int i = 0; i <= ySeg; i++) {
		QBarDataRow* tempDataRow = new QBarDataRow;
		for (int j = 0; j <= xSeg; j++) {
			tempDataRow->append(QBarDataItem(0));
		}
		this->array_Bar_Plot1->append(tempDataRow);
	}
	this->proxy_Bar_Plot1->resetArray(this->array_Bar_Plot1);
	this->bar_Plot1->setBarThickness(xStep / yStep);
}

void pasc::composeCompression() {
	this->curProfileIndex = 2;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);

	ed_currentGCode.clear();
	QStringList cycleGCode = QStringList({ "M302 S0", "M107" });
	cycleGCode.clear();

	double xCenter = ui.lineEdit_CompressionCenterX->text().toDouble();
	double yCenter = ui.lineEdit_CompressionCenterY->text().toDouble();
	double zStep1 = ui.lineEdit_CompressionZStep1->text().toDouble();
	double zStep2 = ui.lineEdit_CompressionZStep2->text().toDouble();
	double fZ = ui.lineEdit_CompressionF->text().toDouble();
	double force1 = ui.lineEdit_CompressionForce1->text().toDouble();
	double force2 = ui.lineEdit_CompressionForce2->text().toDouble();
	double retractZ = ui.lineEdit_CompressionRetractZ->text().toDouble();
	double retractZStep = ui.lineEdit_CompressionRetractZStep->text().toDouble();

	int cycles = ui.lineEdit_CompressionCycles->text().toInt();

	ed_currentGCode.append("ED: G91"); //relative

	this->composeHeaderCharge();
	ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(1000));
	this->composeHeaderChargeReady();



	/*ed_currentGCode.append(QString("ED: G0 Z-" + QString::number(round(zStep1 * 100) / 100) + " F" + QString::number(fZ)));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");
	ed_currentGCode.append(QString("PC: LOOP 4 UNTIL FGREATER " + QString::number(round(force1 * 100) / 100)));
	ed_currentGCode.append(QString("ED: G0 Z-" + QString::number(round(zStep2 * 100) / 100) + " F" + QString::number(fZ)));
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	ed_currentGCode.append(QString("PC: LOOP 4 UNTIL FGREATER " + QString::number(round(force2 * 100) / 100)));*/
	cycleGCode.append(QString("ED: G0 Z-" + QString::number(round(zStep1 * 100) / 100) + " F" + QString::number(fZ)));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode.append(QString("PC: LOOP 4 UNTIL FGREATER " + QString::number(round(force1 * 100) / 100)));
	cycleGCode.append(QString("ED: G0 Z-" + QString::number(round(zStep2 * 100) / 100) + " F" + QString::number(fZ)));
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	cycleGCode << QString("PC: AcquireProfile");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	cycleGCode.append(QString("PC: LOOP 4 UNTIL FGREATER " + QString::number(round(force2 * 100) / 100)));

	// Massage
	//double massageR = 2;
	//int massageN = 5;
	//ed_currentGCode.append(QString("ED: G0 X-" + QString::number(round(massageR * 100) / 100) + " F" + QString::number(fZ)));
	//ed_currentGCode.append(QString("ED: G2 I" + QString::number(round(massageR * 100) / 100) + " J" + QString::number(0)));
	//ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(massageR * 100) / 100) + " F" + QString::number(fZ)));

	//

	//Retract

	/*for (int count = 0; count < retractZ/retractZStep; count++) {
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(retractZStep * 100) / 100) + " F" + QString::number(fZ)));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
	}*/
	for (int count = 0; count < retractZ / retractZStep; count++) {
		cycleGCode.append(QString("ED: G0 Z" + QString::number(round(retractZStep * 100) / 100) + " F" + QString::number(fZ)));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
		cycleGCode << QString("PC: AcquireProfile");
	}

	for (int count = 0; count < cycles; count++) {
		ed_currentGCode.append(cycleGCode);
	}

	ed_currentGCode.append("ED: G90"); //Absolute



	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));

}

void pasc::composeFormalCT() {
	this->curProfileIndex = 6;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);

	ed_currentGCode.clear();
	QStringList cycleGCode = QStringList({ "M302 S0", "M107" });
	cycleGCode.clear();

	double XCenter = ui.lineEdit_FormalCT_XCenter->text().toDouble();
	double YCenter = ui.lineEdit_FormalCT_YCenter->text().toDouble();
	double ZInitial = ui.lineEdit_FormalCT_ZInitial->text().toDouble();
	double XYFeed = ui.lineEdit_FormalCT_XYFeed->text().toDouble();
	double ZFeed = ui.lineEdit_FormalCT_ZFeed->text().toDouble();

	double zStep1 = ui.lineEdit_FormalCT_ZStep1->text().toDouble();
	//double zStep2 = ui.lineEdit_CompressionZStep2->text().toDouble();
	double force1 = ui.lineEdit_FormalCT_Force1->text().toDouble();
	//double force2 = ui.lineEdit_CompressionForce2->text().toDouble();
	double retractZ = ui.lineEdit_FormalCT_Retraction->text().toDouble();
	double retractZStep = ui.lineEdit_FormalCT_RetractionStep->text().toDouble();

	int cycles = ui.lineEdit_FormalCT_CompressionCycles->text().toInt();
	int cyclesMassage = ui.lineEdit_FormalCT_CycleMassage->text().toInt();

	double XRest = ui.lineEdit_FormalCT_XRest->text().toDouble();
	double YRest = ui.lineEdit_FormalCT_YRest->text().toDouble();
	double ZRest = ui.lineEdit_FormalCT_ZRest->text().toDouble();
	int restTime = ui.lineEdit_FormalCT_TransitionWait->text().toInt();
	int restTimeCompression = ui.lineEdit_FormalCT_WaitCompression->text().toInt();
	int restTimeAway = ui.lineEdit_FormalCT_AwayWait->text().toInt();

	double XMassage = ui.lineEdit_FormalCT_XMassage->text().toDouble();
	double YMassage = ui.lineEdit_FormalCT_YMassage->text().toDouble();

	double XTest = ui.lineEdit_FormalCT_XTest->text().toDouble();
	double YTest = ui.lineEdit_FormalCT_YTest->text().toDouble();
	double ZTest = ui.lineEdit_FormalCT_ZTest->text().toDouble();

	double ZTestFinal = ui.lineEdit_FormalCT_ZFinal->text().toDouble();
	double ZTestStep = ui.lineEdit_FormalCT_ZStepTest->text().toDouble();


	// Raise and jog to Start XYZ Position - Compression - from Zero Position - Perform XYZ Zero first
	ed_currentGCode.append("ED: G90"); //absolute
	ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XCenter * 100) / 100) + " Y" + QString::number(round(YCenter * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	// Prepare Electrometer
	this->composeHeaderCharge();
	ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(1000));
	this->composeHeaderChargeReady();
	//ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(1000));
	//ed_currentGCode.append(QString("ED: M114"));
	//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	//if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	//ed_currentGCode << QString("PC: AcquireProfile");
	// Zero Load Cell
	ed_currentGCode << QString("LC: Z");

	for (int count = 0; count < 5 * 5; count++) {
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
	}
	// Initial Testing Cycle To Check Cleaning Efficiency
	if (ui.checkBox_FormalCT_TestTwice->isChecked()) {
		// Go to Z Initial And then Transition
		ed_currentGCode.append("ED: G90"); //absolute
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XRest * 100) / 100) + " Y" + QString::number(round(YRest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		// get continuous sampling
		for (int count = 0; count < restTime * 5; count++) {
			ed_currentGCode.append(QString("ED: M114"));
			//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		}

		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");

		if (!ui.checkBox_FormalCT_CompressionOnly->isChecked()) {
			// Go to test initial position
			ed_currentGCode << QString("LC: Z");
			ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XTest * 100) / 100) + " Y" + QString::number(round(YTest * 100) / 100) + " F" + QString::number(XYFeed)));
			ed_currentGCode.append(QString("ED: M114"));
			ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZTest * 100) / 100) + " F" + QString::number(XYFeed)));
			ed_currentGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");

			// Lower to final test position
			for (double zTemp = ZTest; zTemp >= ZTestFinal; zTemp = zTemp - ZTestStep) {
				ed_currentGCode.append("ED: G0 Z" + QString::number(round(zTemp * 100) / 100) + " F" + QString::number(ZFeed));
				ed_currentGCode.append(QString("ED: M114"));
				ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(200));
				if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
				if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
				ed_currentGCode << QString("PC: AcquireProfile");
			}
			ed_currentGCode.append("ED: G0 Z" + QString::number(round(ZTestFinal * 100) / 100) + " F" + QString::number(ZFeed));
			ed_currentGCode.append(QString("ED: M114"));
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(200));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			// Stay and elapse
			for (int count = 0; count < ui.lineEdit_FormalCT_TimeElapse_1->text().toInt() * 5; count++) {
				ed_currentGCode.append(QString("ED: M114"));
				if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
				if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
				ed_currentGCode << QString("PC: AcquireProfile");
				ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
			}
		}

		// Go to Z Initial And then Transition back
		ed_currentGCode.append("ED: G90"); //absolute
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XRest * 100) / 100) + " Y" + QString::number(round(YRest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		// get continuous sampling
		for (int count = 0; count < restTime * 5; count++) {
			ed_currentGCode.append(QString("ED: M114"));
			//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		}
		ed_currentGCode << QString("LC: Z");
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
	}

	// Compression Cycles:

	// Go to Transition First, to check load cell, and sample the current charge measurement
	cycleGCode.append("ED: G90"); //absolute
	if (ui.checkBox_FormalCT_MoveAway->isChecked()) {
		cycleGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));
		cycleGCode.append(QString("ED: M114"));
		cycleGCode.append(QString("ED: G0 X" + QString::number(round(XRest * 100) / 100) + " Y" + QString::number(round(YRest * 100) / 100) + " F" + QString::number(XYFeed)));
		cycleGCode.append(QString("ED: M114"));
		cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(1 * 1000));
		cycleGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
		cycleGCode << QString("PC: AcquireProfile");
		// get continuous sampling
		for (int count = 0; count < restTimeAway * 5; count++) {
			cycleGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
			cycleGCode << QString("PC: AcquireProfile");
			cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		}
	}
	else {
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XRest * 100) / 100) + " Y" + QString::number(round(YRest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(1 * 1000));
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		// get continuous sampling
		for (int count = 0; count < restTimeAway * 5; count++) {
			ed_currentGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		}
	}

	// Then go back	
	cycleGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));
	cycleGCode.append(QString("ED: M114"));
	if (true) {
		for (int count = 0; count < restTimeAway * 5; count++) {
			ed_currentGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		}
	}
	cycleGCode.append(QString("ED: G0 X" + QString::number(round(XCenter * 100) / 100) + " Y" + QString::number(round(YCenter * 100) / 100) + " F" + QString::number(XYFeed)));
	cycleGCode.append(QString("ED: M114"));
	cycleGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
	cycleGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	cycleGCode << QString("PC: AcquireProfile");
	//ed_currentGCode.append("ED: G91"); //relative
	cycleGCode.append("ED: G91"); //relative
	cycleGCode.append(QString("ED: G0 Z-" + QString::number(round(zStep1 * 100) / 100) + " F" + QString::number(ZFeed)));
	cycleGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode.append(QString("PC: LOOP 5 UNTIL FGREATER " + QString::number(round(force1 * 100) / 100)));
	// wait and massage
	for (int count = 0; count < restTimeCompression * 5; count++) {
		cycleGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
		cycleGCode << QString("PC: AcquireProfile");
		cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
	}
	//cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(restTimeCompression * 1000));
	cycleGCode.append("ED: G90"); // absolute
	if (false) {
		for (int count = 0; count < cyclesMassage; count++) {
			cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter + XMassage) * 100) / 100) + " Y" + QString::number(round((YCenter + YMassage) * 100) / 100) + " F" + QString::number(XYFeed)));
			cycleGCode.append(QString("ED: M114"));
			cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter - XMassage) * 100) / 100) + " Y" + QString::number(round((YCenter + YMassage) * 100) / 100) + " F" + QString::number(XYFeed)));
			cycleGCode.append(QString("ED: M114"));
			cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter - XMassage) * 100) / 100) + " Y" + QString::number(round((YCenter - YMassage) * 100) / 100) + " F" + QString::number(XYFeed)));
			cycleGCode.append(QString("ED: M114"));
			cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter + XMassage) * 100) / 100) + " Y" + QString::number(round((YCenter - YMassage) * 100) / 100) + " F" + QString::number(XYFeed)));
			cycleGCode.append(QString("ED: M114"));
			cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter + XMassage) * 100) / 100) + " Y" + QString::number(round((YCenter + YMassage) * 100) / 100) + " F" + QString::number(XYFeed)));
			cycleGCode.append(QString("ED: M114"));
		}
		cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter + 0) * 100) / 100) + " Y" + QString::number(round((YCenter + 0) * 100) / 100) + " F" + QString::number(XYFeed)));
		cycleGCode.append(QString("ED: M114"));
	}
	else {
		for (int count = 0; count < cyclesMassage; count++) {
			for (double tempTheta = 0; tempTheta < 4 * acos(0.0); tempTheta = tempTheta + acos(0.0) / 10) {
				cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter + XMassage * cos(tempTheta)) * 100) / 100) + " Y" + QString::number(round((YCenter + XMassage * sin(tempTheta)) * 100) / 100) + " F" + QString::number(XYFeed)));
				cycleGCode.append(QString("ED: M114"));
				if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
				if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
				cycleGCode << QString("PC: AcquireProfile");
			}
		}
		cycleGCode.append(QString("ED: G0 X" + QString::number(round((XCenter + 0) * 100) / 100) + " Y" + QString::number(round((YCenter + 0) * 100) / 100) + " F" + QString::number(XYFeed)));
		cycleGCode.append(QString("ED: M114"));
	}
	cycleGCode.append("ED: G91"); //relative
	for (int count = 0; count < retractZ / retractZStep; count++) {
		cycleGCode.append(QString("ED: G0 Z" + QString::number(round(retractZStep * 100) / 100) + " F" + QString::number(ZFeed)));
		cycleGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
		cycleGCode << QString("PC: AcquireProfile");
	}

	for (int count = 0; count < cycles; count++) {
		ed_currentGCode.append(cycleGCode);
	}

	// Go to Z Initial And then Transition
	ed_currentGCode.append("ED: G90"); //absolute
	ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));

	// Wait for Protect Plate 
	for (int count = 0; count < restTime * 2; count++) {
		ed_currentGCode.append(QString("ED: M114"));
		//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
	}

	ed_currentGCode.append(QString("ED: M114"));
	ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XRest * 100) / 100) + " Y" + QString::number(round(YRest * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");
	// get continuous sampling
	for (int count = 0; count < restTime * 5; count++) {
		ed_currentGCode.append(QString("ED: M114"));
		//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
	}

	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	if (!ui.checkBox_FormalCT_CompressionOnly->isChecked()) {
		// Go to test initial position
		ed_currentGCode << QString("LC: Z");
		ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XTest * 100) / 100) + " Y" + QString::number(round(YTest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZTest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");

		// Lower to final test position
		for (double zTemp = ZTest; zTemp >= ZTestFinal; zTemp = zTemp - ZTestStep) {
			ed_currentGCode.append("ED: G0 Z" + QString::number(round(zTemp * 100) / 100) + " F" + QString::number(ZFeed));
			ed_currentGCode.append(QString("ED: M114"));
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(200));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
		}
		ed_currentGCode.append("ED: G0 Z" + QString::number(round(ZTestFinal * 100) / 100) + " F" + QString::number(ZFeed));
		ed_currentGCode.append(QString("ED: M114"));
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(200));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		// Stay and elapse
		//int targetTime = ui.lineEdit_FormalCT_TimeElapse->text().toInt();
		//ed_currentGCode << QString("PC: SampleTimeOn");
		//ed_currentGCode << QString("ED: M114");
		//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		//if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		//ed_currentGCode << QString("PC: AcquireProfile");
		//ed_currentGCode.append(QString("PC: LOOP 4 UNTIL TGREATER " + QString::number(targetTime)));
		//ed_currentGCode << QString("PC: SampleTimeOff");
		//ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZTest * 100) / 100) + " F" + QString::number(XYFeed)));
		//ed_currentGCode.append(QString("ED: M114"));
		for (int count = 0; count < ui.lineEdit_FormalCT_TimeElapse->text().toInt() * 5; count++) {
			ed_currentGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		}
		ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZTest * 100) / 100) + " F" + QString::number(XYFeed)));
		ed_currentGCode.append(QString("ED: M114"));
	}
	// Finalize at transition location - Charge Collection Temporary
	ed_currentGCode.append("ED: G90"); //absolute
	//ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
	//ed_currentGCode.append(QString("ED: M114"));
	ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZRest * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	ed_currentGCode.append(QString("ED: G0 X" + QString::number(round(XRest * 100) / 100) + " Y" + QString::number(round(YRest * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");
	// get continuous sampling
	for (int count = 0; count < restTime * 5 * 3; count++) {
		ed_currentGCode.append(QString("ED: M114"));
		//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
	}
	// Finalizing
	ed_currentGCode.append(QString("ED: G0 Z" + QString::number(round(ZInitial * 100) / 100) + " F" + QString::number(XYFeed)));
	ed_currentGCode.append(QString("ED: M114"));
	ed_currentGCode.append("ED: G90"); //absolute
	ed_currentGCode << QString("ED: M114");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::composeKirigami() {
	this->curProfileIndex = 7;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);

	ed_currentGCode.clear();
	QStringList cycleGCode = QStringList({ "M302 S0", "M107" });
	cycleGCode.clear();

	double ZTarget = ui.lineEdit_Kirigami_Ztarget->text().toDouble();
	double ZStep = ui.lineEdit_Kirigami_ZStep->text().toDouble();
	int cycles = ui.lineEdit_Kirigami_Cycles->text().toInt();

	ZStep = abs(ZStep);
	int ZSteps = floor(abs(ZTarget) / abs(ZStep));

	ed_currentGCode.append("ED: G90");
	ed_currentGCode << QString("LC: Z");

	for (int count = 0; count < ZSteps; count++) {
		cycleGCode.append("ED: G91");
		if (ZTarget >= 0) {
			cycleGCode.append(QString("ED: G0 Z" + QString::number(round(ZStep * 100) / 100) + " F" + QString::number(200)));
		}
		else {
			cycleGCode.append(QString("ED: G0 Z-" + QString::number(round(ZStep * 100) / 100) + " F" + QString::number(200)));
		}
		cycleGCode.append("ED: G90");
		cycleGCode.append(QString("ED: M114"));
		cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
		cycleGCode << QString("PC: AcquireProfile");
	}
	cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(5000));
	for (int count = 0; count < ZSteps; count++) {
		cycleGCode.append("ED: G91");
		if (ZTarget >= 0) {
			cycleGCode.append(QString("ED: G0 Z-" + QString::number(round(ZStep * 100) / 100) + " F" + QString::number(200)));
		}
		else {
			cycleGCode.append(QString("ED: G0 Z" + QString::number(round(ZStep * 100) / 100) + " F" + QString::number(200)));
		}
		cycleGCode.append("ED: G90");
		cycleGCode.append(QString("ED: M114"));
		cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(0.2 * 1000));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
		cycleGCode << QString("PC: AcquireProfile");
	}
	cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(5000));

	for (int count = 0; count < cycles; count++) {
		ed_currentGCode.append(cycleGCode);
	}

	ed_currentGCode.append("ED: G90"); //absolute
	ed_currentGCode << QString("ED: M114");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	ed_currentGCode << QString("PC: AcquireProfile");
	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::composeCTFutek() {
	this->curProfileIndex = 8;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);

	ed_currentGCode.clear();
	QStringList cycleGCode = QStringList({ "M302 S0", "M107" });
	cycleGCode.clear();

	//double ZInitial = ui.lineEdit_FormalCT_ZInitial->text().toDouble();
	//double zStep1 = ui.lineEdit_FormalCT_ZStep1->text().toDouble();
	int numActiveSensors = ui.checkBox_LinearScanLoadCellEnabled->isChecked() + ui.checkBox_LinearScanElectrometerEnabled->isChecked() + ui.checkBox_FutekEnabled->isChecked();

	int cycles = ui.lineEdit_CTFutek_CompCycles->text().toInt();
	double zStep1 = ui.lineEdit_CTFutek_CompressionStep->text().toDouble();
	double force1 = ui.lineEdit_CTFutek_CompressionForce->text().toDouble();
	int ZFeed = ui.lineEdit_CTFutek_ZFeed->text().toInt();
	double compWait = ui.lineEdit_CTFutek_CompressionWait->text().toDouble();
	double destWait = ui.lineEdit_CTFutek_DestinationWait->text().toDouble();
	double backZPos = ui.lineEdit_CTFutek_BackZPosition->text().toDouble();
	double backWait = ui.lineEdit_CTFutek_BackWait->text().toDouble();
	double zStep2 = ui.lineEdit_CTFutek_BackZStep->text().toDouble();
	double detachWait = ui.lineEdit_CTFutek_DetachWait->text().toDouble();

	//double retractZ = 2.5;

	// Prepare Electrometer
	this->composeHeaderVoltage();
	ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(5000));

	// Zero Load Cell
	//if (ui.checkBox_VacuumTest1_ResetLoadCell->isChecked())	ed_currentGCode << QString("LC: Z");

	// Compression Cycles:
	cycleGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) cycleGCode << QString("FT: MEAS");
	cycleGCode << QString("PC: AcquireProfile");

	cycleGCode.append("ED: G91"); //relative
	cycleGCode.append(QString("ED: M114"));

	cycleGCode.append(QString("ED: G0 Z-" + QString::number(round(zStep1 * 100) / 100) + " F" + QString::number(ZFeed)));
	cycleGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) cycleGCode << QString("FT: MEAS");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(compWait * 1000));
	cycleGCode.append(QString("PC: LOOP " + QString::number(numActiveSensors + 4) + " UNTIL FGREATER " + QString::number(round(force1 * 100) / 100)));

	// hold for a while
	// get continuous sampling
	cycleGCode << QString("ED: M114");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) cycleGCode << QString("FT: MEAS");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode.append(QString("PC: LOOP " + QString::number(numActiveSensors + 2) + " UNTIL TGREATER " + QString::number(destWait)));

	// rub
	
	if (ui.checkBox_CTFutek_Rub->isChecked()) {
		double eRub = ui.lineEdit_CTFutek_Rub->text().toDouble();
		cycleGCode.append("ED: G91"); //relative
		cycleGCode.append(QString("ED: M114"));
		cycleGCode.append(QString("ED: G0 E-" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
		//cycleGCode.append(QString("ED: G0 E" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
		//cycleGCode.append(QString("ED: G0 E" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
		//cycleGCode.append(QString("ED: G0 E-" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
	}




	// Retract
	cycleGCode.append("ED: G90"); // absolute

	cycleGCode.append("ED: G91"); //relative
	cycleGCode.append(QString("ED: M114"));

	cycleGCode.append(QString("ED: G0 Z" + QString::number(round(zStep2 * 100) / 100) + " F" + QString::number(ZFeed)));
	cycleGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) cycleGCode << QString("FT: MEAS");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(backWait * 1000));
	cycleGCode.append(QString("PC: LOOP " + QString::number(numActiveSensors + 4) + " UNTIL ZGREATER " + QString::number(round(backZPos * 100) / 100)));

	// rub back
	if (ui.checkBox_CTFutek_Rub->isChecked()) {
		double eRub = ui.lineEdit_CTFutek_Rub->text().toDouble();
		cycleGCode.append("ED: G91"); //relative
		cycleGCode.append(QString("ED: M114"));
		cycleGCode.append(QString("ED: G0 E" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
		//cycleGCode.append(QString("ED: G0 E" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
		//cycleGCode.append(QString("ED: G0 E" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
		//cycleGCode.append(QString("ED: G0 E-" + QString::number(round(eRub * 100) / 100) + " F" + QString::number(50)));
	}


	// hold for a while
	// get continuous sampling
	cycleGCode << QString("ED: M114");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) cycleGCode << QString("FT: MEAS");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode.append(QString("PC: LOOP " + QString::number(numActiveSensors + 2) + " UNTIL TGREATER " + QString::number(detachWait)));


	cycleGCode.append("ED: G90"); //absolute


	for (int count = 0; count < cycles; count++) {
		ed_currentGCode.append(cycleGCode);
	}

	// Final Lift 
	double liftZPos = ui.lineEdit_CTFutek_FinalLiftHeight->text().toDouble();
	double liftStep = ui.lineEdit_CTFutek_FinalLiftStep->text().toDouble();
	double liftWait = ui.lineEdit_CTFutek_FinalLiftWait->text().toDouble();
	cycleGCode.clear();

	cycleGCode.append("ED: G91"); //relative
	cycleGCode.append(QString("ED: M114"));


	cycleGCode.append(QString("ED: M17")); // Enable Stepper
	cycleGCode.append(QString("ED: M114"));
	cycleGCode.append(QString("ED: G0 Z" + QString::number(round(liftStep * 100) / 100) + " F" + QString::number(ZFeed)));
	cycleGCode.append(QString("ED: M114"));
	cycleGCode.append(QString("ED: M18")); // Disable Steppers to eliminate noise
	cycleGCode.append(QString("ED: M114"));
	cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(1 * 1000)); // Wait 1 s for stepper noise to vanish
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) cycleGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) cycleGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) cycleGCode << QString("FT: MEAS");
	cycleGCode << QString("PC: AcquireProfile");
	cycleGCode << QString("PC: DELAYMILISECONDS " + QString::number(liftWait * 1000));
	cycleGCode.append(QString("PC: LOOP " + QString::number(numActiveSensors + 9) + " UNTIL ZGREATER " + QString::number(round(liftZPos * 100) / 100)));

	cycleGCode.append(QString("ED: M17")); // Enable Stepper
	cycleGCode.append(QString("ED: M114"));
	cycleGCode.append("ED: G90"); 
	cycleGCode.append(QString("ED: M114"));

	if (ui.checkBox_CTFutek_FinalLift->isChecked()) ed_currentGCode.append(cycleGCode);



	// Finishing
	ed_currentGCode.append("ED: G90"); //absolute
	ed_currentGCode << QString("ED: M114");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) ed_currentGCode << QString("FT: MEAS");
	ed_currentGCode << QString("PC: AcquireProfile");

	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::composeTimeElapse() {
	this->curProfileIndex = 3;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);

	ed_currentGCode.clear();

	int targetTime = ui.lineEdit_TimeElapseTotalTime->text().toInt();

	//if (ui.checkBox_LinearScanElectrometerEnabled->isChecked() && ui.comboBox_TimeElapseEMMode->currentIndex() == 0) {
	//	this->composeHeaderCharge();
	//}
	//if (ui.checkBox_LinearScanElectrometerEnabled->isChecked() && ui.comboBox_TimeElapseEMMode->currentIndex() == 1) {
	//	this->composeHeaderVoltage();
	//}

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked() && ui.comboBox_TimeElapseEMMode->currentText().contains("Charge")) {
		this->composeHeaderCharge();
	}
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked() && ui.comboBox_TimeElapseEMMode->currentText().contains("Voltage")) {
		this->composeHeaderVoltage();
	}

	ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(1000));
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked() && ui.comboBox_TimeElapseEMMode->currentIndex() == 0) {
		this->composeHeaderChargeReady();
	}

	ed_currentGCode.append(QString("ED: M18")); // Disable Stepper
	ed_currentGCode.append(QString("ED: M114"));

	ed_currentGCode << QString("ED: M114");
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	if (ui.checkBox_FutekEnabled->isChecked()) ed_currentGCode << QString("FT: MEAS");
	ed_currentGCode << QString("PC: AcquireProfile");
	int numActiveSensors = ui.checkBox_LinearScanLoadCellEnabled->isChecked() + ui.checkBox_LinearScanElectrometerEnabled->isChecked() + ui.checkBox_FutekEnabled->isChecked();
	ed_currentGCode.append(QString("PC: LOOP " + QString::number(numActiveSensors+2) + " UNTIL TGREATER " + QString::number(targetTime)));

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		ed_currentGCode << QString("EM: SYST:ZCH ON");
	}

	ed_currentGCode.append(QString("ED: M17")); // Enable Stepper
	ed_currentGCode.append(QString("ED: M114"));


	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::composeChargeCollect() {
	this->curProfileIndex = 4;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);
	ed_currentGCode.clear();

	ed_currentGCode << QString("ED: M302 S0");
	ed_currentGCode << QString("ED: M92 E") + ui.lineEdit_Header_EStep->text();
	ed_currentGCode << QString("ED: G92 E") + ui.lineEdit_ChargeCollectEInitial->text();

	double eStart = ui.lineEdit_ChargeCollectEStart->text().toDouble();
	int eFeed = ui.lineEdit_ChargeCollectEFeed->text().toInt();
	double eEnd = ui.lineEdit_ChargeCollectEEnd->text().toDouble();
	double yStart = ui.lineEdit_ChargeCollectYStart->text().toDouble();
	double yEnd = ui.lineEdit_ChargeCollectYEnd->text().toDouble();
	double yStep = ui.lineEdit_ChargeCollectYStep->text().toDouble();
	int yFeed = ui.lineEdit_ChargeCollectYFeed->text().toInt();
	double zRaise = ui.lineEdit_ChargeCollectZRaise->text().toDouble();
	int zFeed = ui.lineEdit_ChargeCollectZFeed->text().toInt();
	double eInitial = ui.lineEdit_ChargeCollectEInitial->text().toDouble();
	double zInitial = ui.lineEdit_ChargeCollectZInitial->text().toDouble();
	int delayTimeMS = 1000;
	int eCycles = ui.lineEdit_ChargeCollectECycles->text().toInt();

	this->singleReadXYZE();
	double yInitial = this->curXYZE[1];

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		this->composeHeaderCharge();
	}

	ed_currentGCode.append("ED: G90");

	ed_currentGCode.append("ED: G0 Z" + QString::number(round(zInitial * 100) / 100) + " F" + QString::number(zFeed));
	ed_currentGCode.append(QString("ED: M114"));

	ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(delayTimeMS));

	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");



	ed_currentGCode.append("ED: G0 Z" + QString::number(round((zInitial + zRaise) * 100) / 100) + " F" + QString::number(zFeed));
	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	//ed_currentGCode.append("ED: G91"); //relative
	//ed_currentGCode.append("ED: G0 Z" + QString::number(round(zRaise*100) / 100) + " F" + QString::number(zFeed));

	/*ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");*/

	//ed_currentGCode.append("ED: G90");

	ed_currentGCode.append("ED: G0 E" + QString::number(round(eStart * 100) / 100) + " F" + QString::number(eFeed));
	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		this->composeHeaderChargeReady();
	}

	for (int i = 0; i <= ceil((yEnd - yStart) / yStep); i++) {
		ed_currentGCode.append("ED: G0 Y" + QString::number(round(100 * (yStart + i * yStep)) / 100) + " F" + QString::number(yFeed));
		ed_currentGCode.append(QString("ED: M114"));
		if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
		if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
		ed_currentGCode << QString("PC: AcquireProfile");
		for (int j = 0; j < eCycles; j++) {
			ed_currentGCode.append("ED: G0 E" + QString::number(round(eEnd * 100) / 100) + " F" + QString::number(eFeed));
			ed_currentGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
			ed_currentGCode.append("ED: G0 E" + QString::number(round(eStart * 100) / 100) + " F" + QString::number(eFeed));
			ed_currentGCode.append(QString("ED: M114"));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
		}
	}

	ed_currentGCode.append("ED: G0 E" + QString::number(round(eInitial * 100) / 100) + " F" + QString::number(eFeed));
	ed_currentGCode.append(QString("ED: M114"));

	ed_currentGCode.append("ED: G0 Y" + QString::number(round(yInitial * 100) / 100) + " F" + QString::number(yFeed));
	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	ed_currentGCode.append("ED: G0 Z" + QString::number(round(zInitial * 100) / 100) + " F" + QString::number(zFeed));
	ed_currentGCode.append(QString("ED: M114"));

	ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(delayTimeMS));

	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	ed_currentGCode.append("ED: G0 Z" + QString::number(round((zInitial + zRaise) * 100) / 100) + " F" + QString::number(zFeed));
	ed_currentGCode.append(QString("ED: M114"));
	if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	ed_currentGCode << QString("PC: AcquireProfile");

	//ed_currentGCode.append("ED: G91"); //relative
	//ed_currentGCode.append("ED: G0 Z" + QString::number(-round(zRaise * 100) / 100) + " F" + QString::number(zFeed));
	//ed_currentGCode.append(QString("ED: M114"));
	//if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
	//if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
	//ed_currentGCode << QString("PC: AcquireProfile");

	ed_currentGCode.append("ED: G90");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		ed_currentGCode << QString("EM: SYST:ZCH ON");
	}

	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::composeZProbe() {
	this->curProfileIndex = 5;
	ui.comboBox_CurrentProfile->setCurrentIndex(this->curProfileIndex);
	ed_currentGCode.clear();

	this->singleReadXYZE();
	double zInitial = this->curXYZE[2];
	double zEnd1 = ui.lineEdit_ZProbeZend1->text().toDouble();
	double zEnd2 = ui.lineEdit_ZProbeZend2->text().toDouble();
	double zStep1 = ui.lineEdit_ZProbeZstep1->text().toDouble();
	double zStep2 = ui.lineEdit_ZProbeZstep2->text().toDouble();
	int zFeed = ui.lineEdit_ZProbeZfeed->text().toInt();
	double zStart = ui.lineEdit_ZProbeZStart->text().toDouble();

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		this->composeHeaderCharge();
	}

	ed_currentGCode.append("ED: G90");

	ed_currentGCode.append("ED: G0 Z" + QString::number(round(zStart * 100) / 100) + " F" + QString::number(zFeed));
	ed_currentGCode.append(QString("ED: M114"));

	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		ed_currentGCode << QString("EM: SYST:ZCH OFF");
		ed_currentGCode << QString("EM: SYST:ZCOR ON");
		ed_currentGCode << QString("EM: CALC2:NULL:STAT ON");
		ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(2000));
		ed_currentGCode << QString("EM: READ?");
	}

	int delayTimeMS = 200;
	int cycles = ui.lineEdit_ZProbeCycle->text().toInt();

	for (int k = 0; k < cycles; k++) {
		for (double zTemp = zStart; zTemp >= zEnd1; zTemp = zTemp - zStep1) {
			ed_currentGCode.append("ED: G0 Z" + QString::number(round(zTemp * 100) / 100) + " F" + QString::number(zFeed));
			ed_currentGCode.append(QString("ED: M114"));
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(delayTimeMS));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
		}
		for (double zTemp = zEnd1 - zStep2; zTemp >= zEnd2; zTemp = zTemp - zStep2) {
			ed_currentGCode.append("ED: G0 Z" + QString::number(round(zTemp * 100) / 100) + " F" + QString::number(zFeed));
			ed_currentGCode.append(QString("ED: M114"));
			ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(delayTimeMS));
			if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
			if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
			ed_currentGCode << QString("PC: AcquireProfile");
		}
		if (ui.checkBox_ZProbeCycle->isChecked()) {
			for (double zTemp = zEnd2; zTemp <= zEnd1; zTemp = zTemp + zStep2) {
				ed_currentGCode.append("ED: G0 Z" + QString::number(round(zTemp * 100) / 100) + " F" + QString::number(zFeed));
				ed_currentGCode.append(QString("ED: M114"));
				ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(delayTimeMS));
				if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
				if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
				ed_currentGCode << QString("PC: AcquireProfile");
			}
			for (double zTemp = zEnd1 + zStep1; zTemp <= zStart; zTemp = zTemp + zStep1) {
				ed_currentGCode.append("ED: G0 Z" + QString::number(round(zTemp * 100) / 100) + " F" + QString::number(zFeed));
				ed_currentGCode.append(QString("ED: M114"));
				ed_currentGCode << QString("PC: DELAYMILISECONDS " + QString::number(delayTimeMS));
				if (ui.checkBox_LinearScanLoadCellEnabled->isChecked()) ed_currentGCode << QString("LC: ?C");
				if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) ed_currentGCode << QString("EM: READ?");
				ed_currentGCode << QString("PC: AcquireProfile");
			}
		}
	}

	ed_currentGCode.append("ED: G0 Z" + QString::number(round(zInitial * 100) / 100) + " F" + QString::number(zFeed));
	ed_currentGCode.append(QString("ED: M114"));

	ed_currentGCode.append("ED: G90");
	if (ui.checkBox_LinearScanElectrometerEnabled->isChecked()) {
		ed_currentGCode << QString("EM: SYST:ZCH ON");
	}

	ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
}

void pasc::saveCurrentProfile() {
	QString tempFileName = QFileDialog::getSaveFileName(this, tr("Save Profile"), "C:/", tr("All Files(*)"));
	if (tempFileName.isEmpty()) {
		return;
	}
	else {
		QFile tempFile(tempFileName);
		if (!tempFile.open(QIODevice::WriteOnly)) {
			QMessageBox::information(this, tr("Unable to open file"), tempFile.errorString());
			return;
		}
		QTextStream tempOut(&tempFile);
		//QDataStream tempOut(&tempFile);
		//tempOut.setVersion(QDataStream::Qt_5_10);
		tempOut << ed_currentGCode.join("\n");
	}
}

void pasc::loadProfile() {
	QString tempFileName = QFileDialog::getOpenFileName(this, tr("Open Profile"), "C:/", tr("All Files(*)"));
	if (tempFileName.isEmpty()) {
		return;
	}
	else {
		QFile tempFile(tempFileName);
		if (!tempFile.open(QIODevice::ReadOnly)) {
			QMessageBox::information(this, tr("Unable to open file"), tempFile.errorString());
			return;
		}
		QTextStream tempIn(&tempFile);
		//QString tempReadProfile;
		//tempIn >> tempReadProfile;
		ed_currentGCode = tempIn.readAll().split("\n");
		ui.textBrowser_CurrenGCode->setText(ed_currentGCode.join("\n"));
	}
}

void pasc::saveCurData() {
	QString tempFileName = QFileDialog::getSaveFileName(this, tr("Save Profile"), "C:/filename.csv", tr("CSV files (.csv)"));
	if (tempFileName.isEmpty()) {
		return;
	}
	else {
		QFile tempFile(tempFileName);
		if (!tempFile.open(QIODevice::WriteOnly)) {
			QMessageBox::information(this, tr("Unable to open file"), tempFile.errorString());
			return;
		}
		QTextStream tempOut(&tempFile);
		//QDataStream tempOut(&tempFile);
		//tempOut.setVersion(QDataStream::Qt_5_10);
		tempOut << "X\t";
		tempOut << "Y\t";
		tempOut << "Z\t";
		tempOut << "T\t";
		tempOut << "E\t";
		tempOut << "F\t";
		tempOut << "K\t";
		tempOut << "P\n";
		for (int i = 0; i < this->data_Time.length(); i++) {
			tempOut << data_X[i] << "\t";
			tempOut << data_Y[i] << "\t";
			tempOut << data_Z[i] << "\t";
			tempOut << data_Time[i] << "\t";
			tempOut << data_Elec[i] << "\t";
			tempOut << data_Force[i] << "\t";
			tempOut << data_Futek[i] << "\t";
			tempOut << data_Pressure[i] << "\n";
		}
		if (this->curProfileIndex == 3 || this->curProfileIndex == 8) {
			tempOut << ui.lineEdit_FutekZero->text().toDouble() << "\t";
			tempOut << ui.lineEdit_FutekZero_2->text().toDouble() << "\t";
			tempOut << 0 << "\t";
			tempOut << 0 << "\t";
			tempOut << 0 << "\t";
			tempOut << 0 << "\t";
			tempOut << 0 << "\t";
			tempOut << 0 << "\n";
		}
	}



	QString tempScreenShotName = tempFileName + ".png";
	QPixmap pixmap = this->grab();
	if (!pixmap.isNull())	pixmap.save(tempScreenShotName, "PNG");
}


void pasc::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == this->timer_Read1)
	{
		killTimer(this->timer_Read1);
		if (!this->profileHalted) this->sendMessage(ed_currentGCode.at(curCodeIndex), 1);
		if (this->profileHalted) {
			QByteArray endCode("ok");
			QString singleMessage("");
			singleMessage = "ED: G90";
			//singleMessage = this->serialPrefixes.at(ui.comboBox_SendTarget->currentIndex()) + singleMessage;
			//switch (ui.comboBox_SendTarget->currentIndex()) {
			//case 0: singleMessage = this->serialPrefixes.at(0) + singleMessage;
			//	break;
			//case 1: singleMessage = this->serialPrefixes.at(1) + singleMessage;
			//	break;
			//}
			ui.lineEdit_SendMessage->setText(singleMessage);
			this->sendMessage(singleMessage, 0);
			isLoopTiming = false;
		}

		//QByteArray endCode("ok");
		//for (int tempCount = 0; tempCount < ed_currentGCode.length(); tempCount++) {
		//	sendMessage(ed_currentGCode.at(tempCount).toUtf8(), endCode);
		//}
	}
	if (event->timerId() == this->timer_LC1) {
		killTimer(this->timer_LC1);
		//QByteArray endCode = "\r\n";
		//QByteArray feedbackMessage = lc_serialPort->readLine();
		//while (feedbackMessage.isEmpty() || !feedbackMessage.contains(endCode)) {
		//	ed_serialPort->waitForReadyRead(1);
		//	feedbackMessage.append(lc_serialPort->readLine());
		//	//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
		//}
		//qDebug() << "received >> " << feedbackMessage;
		//lc_currentFeedback = QString(feedbackMessage);
	}
	if (event->timerId() == this->timer_ED1) {
		killTimer(this->timer_ED1);
		//QByteArray endCode1;
		//QByteArray endCode2;
		//if (message.contains("INITIALIZE")) {
		//	message.remove("INITIALIZE");
		//	endCode = "size: \n585";
		//}
		//else
		//{
		//	endCode = "ok";
		//}
		//QByteArray feedbackMessage = this->readMessage();
		//while (feedbackMessage.isEmpty() || !(feedbackMessage.contains(ed_CurEndCode))) {
		//	ed_serialPort->waitForReadyRead(1);
		//	feedbackMessage.append(this->readMessage());
		//	//ui.textBrowser_ReadHistory->setText(QString(feedbackMessage));
		//}
		////ui.textBrowser_ReadHistory->append(QString(feedbackMessage));
		////this->update();
		//ed_currentFeedback = QString(feedbackMessage);
	}

}

void pasc::lcDubug() {
	//this->sendMessage(ui.lineEdit_LCDebug->text(), 0);
	//this->sendMessage("LC: ?", 0);
	//QString message = ui.lineEdit_EMDEBUG->text();
	//message.prepend("EM: ");
	//this->sendMessage(message, 0);
	ui.lineEdit_EMDEBUG->setText("EM: 'TIME'");
	QString message = "EM: FUNC 'CHAR'";
	this->sendMessage(message, 0);
}

void pasc::stopReading() {
	killTimer(this->timer_Read1);
}

void pasc::incrementXP() {
	if (!jogReclickLock) {
		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[0] + ui.lineEdit_XInc->text().toDouble();
		this->sendMessage("ED: G0 X" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}
}

void pasc::incrementYP() {
	if (!jogReclickLock) {
		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[1] + ui.lineEdit_YInc->text().toDouble();
		this->sendMessage("ED: G0 Y" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}

}

void pasc::incrementZP() {
	if (!jogReclickLock) {
		/*this->haltCurrentProfile();*/

		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[2] + ui.lineEdit_ZInc->text().toDouble();
		this->sendMessage("ED: G0 Z" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}
}

void pasc::incrementEP() {
	if (!jogReclickLock) {
		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[3] + ui.lineEdit_EInc->text().toDouble();
		this->sendMessage("ED: G0 E" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}

}

void pasc::incrementXM() {
	if (!jogReclickLock) {
		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[0] - ui.lineEdit_XInc->text().toDouble();
		if (destination < 0) destination = 0;
		this->sendMessage("ED: G0 X" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}

}

void pasc::incrementYM() {
	if (!jogReclickLock) {
		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[1] - ui.lineEdit_YInc->text().toDouble();
		if (destination < 0) destination = 0;
		this->sendMessage("ED: G0 Y" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}

}

void pasc::incrementZM() {
	if (!jogReclickLock) {
		/*this->haltCurrentProfile();*/

		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[2] - ui.lineEdit_ZInc->text().toDouble();
		//if (destination < 0) destination = 0;
		this->sendMessage("ED: G0 Z" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}

}

void pasc::incrementEM() {
	if (!jogReclickLock) {
		jogReclickLock = true;
		this->singleReadXYZE();
		double destination = this->curXYZE[3] - ui.lineEdit_EInc->text().toDouble();
		if (destination < 0) destination = 0;
		this->sendMessage("ED: G0 E" + QString::number(destination) + " F" + ui.lineEdit_IncSpeed->text(), 0);
		this->singleReadXYZE();
		jogReclickLock = false;
	}

}

void pasc::homeXWhenSafe() {
	if (ui.checkBox_XSafe->isChecked()) {
		this->sendMessage("ED: G28 X", 0);
		ui.checkBox_XSafe->setChecked(0);
	}
}

void pasc::homeYWhenSafe() {
	if (ui.checkBox_YSafe->isChecked()) {
		this->sendMessage("ED: G28 Y", 0);
		ui.checkBox_YSafe->setChecked(0);
	}
}

void pasc::homeZWhenSafe() {
	if (ui.checkBox_ZSafe->isChecked()) {
		this->sendMessage("ED: G28 Z", 0);
		ui.checkBox_ZSafe->setChecked(0);
	}
}

void pasc::disableEndStop() {
	qDebug() << "Disabling EndStops";
	this->sendMessage("ED: M211 S0", 0);
	this->sendMessage("ED: M114", 0);
	this->sendMessage("ED: M302 S0", 0);
	
}

void pasc::zeroFutek() {
	ui.lineEdit_FutekZero->setText(QString::number(curFutekReading));
}
