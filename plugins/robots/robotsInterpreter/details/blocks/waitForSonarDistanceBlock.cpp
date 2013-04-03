#include "waitForSonarDistanceBlock.h"

#include "../../sensorConstants.h"

using namespace qReal;
using namespace interpreters::robots;
using namespace interpreters::robots::details::blocks;

WaitForSonarDistanceBlock::WaitForSonarDistanceBlock(details::RobotModel const * const robotModel)
	: mRobotModel(robotModel)
{
	// There is about 30 ms latency within robot bluetooth chip, so it is useless to
	// read sensor too frequently.
	mActiveWaitingTimer.setInterval(100);

	connect(&mActiveWaitingTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}

void WaitForSonarDistanceBlock::run()
{
	inputPort::InputPortEnum const port = static_cast<inputPort::InputPortEnum>(intProperty("Port") - 1);
	robotParts::SonarSensor * const sonarSensor = mRobotModel->sonarSensor(port);

	if (!sonarSensor) {
		mActiveWaitingTimer.stop();
		error(tr("Sonar sensor is not configured on this port"));
		return;
	}

	connect(sonarSensor->sensorImpl(), SIGNAL(response(int)), this, SLOT(responseSlot(int)));
	connect(sonarSensor->sensorImpl(), SIGNAL(failure()), this, SLOT(failureSlot()));

	sonarSensor->read();
	mActiveWaitingTimer.start();
}

void WaitForSonarDistanceBlock::stop()
{
	mActiveWaitingTimer.stop();
	emit done(mNextBlock);
}

void WaitForSonarDistanceBlock::responseSlot(int reading)
{
	int const targetDistance = evaluate("Distance").toInt();
	QString const sign = stringProperty("Sign");
	if (sign == "равно")
		if (reading != targetDistance)
			stop();
	if (sign == "больше")
		if (reading <= targetDistance)
			stop();
	if (sign == "меньше")
		if (reading >= targetDistance)
			stop();
	if (sign == "не меньше")
		if (reading < targetDistance)
			stop();
	if (sign == "не больше")
		if (reading > targetDistance)
			stop();
	if (reading < targetDistance)
		stop();
}

void WaitForSonarDistanceBlock::failureSlot()
{
	mActiveWaitingTimer.stop();
	emit failure();
}

void WaitForSonarDistanceBlock::timerTimeout()
{
	// Without the next two lines it fails with segfault with some great probability
	inputPort::InputPortEnum const port = static_cast<inputPort::InputPortEnum>(intProperty("Port") - 1);
	robotParts::SonarSensor * const sonarSensor = mRobotModel->sonarSensor(port);

	if (sonarSensor) {
		sonarSensor->read();
	}
}

QList<Block::SensorPortPair> WaitForSonarDistanceBlock::usedSensors() const
{
	inputPort::InputPortEnum const port = static_cast<inputPort::InputPortEnum>(intProperty("Port") - 1);
	return QList<SensorPortPair>() << qMakePair(sensorType::sonar, static_cast<int>(port));
}

void WaitForSonarDistanceBlock::stopActiveTimerInBlock()
{
	mActiveWaitingTimer.stop();
}
