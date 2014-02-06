#include "graphicsWatcherManager.h"

using namespace interpreterCore;
using namespace utils;
using namespace interpreterBase::robotModel;

GraphicsWatcherManager::GraphicsWatcherManager(ExpressionsParser * const parser, QObject *parent)
	: QObject(parent)
	, mWatcher(new sensorsGraph::SensorsGraph(parser))
{
}

QWidget *GraphicsWatcherManager::widget()
{
	return mWatcher;
}

void GraphicsWatcherManager::forceStart()
{
	mWatcher->startJob();
}

void GraphicsWatcherManager::forceStop()
{
	mWatcher->stopJob();
}

void GraphicsWatcherManager::onSensorConfigurationChanged(QString const &robotModel
		, PortInfo const &port, PluggableDeviceInfo const &sensor)
{
	Q_UNUSED(port)
	Q_UNUSED(sensor)

	updateSensorsList(robotModel);
}

void GraphicsWatcherManager::updateSensorsList(QString const &currentRobotModel)
{
	mWatcher->clearTrackingObjects();
	int index = 0;
	for (PortInfo const &port : mCurrentConfiguration[currentRobotModel].keys()) {
		PluggableDeviceInfo const device = mCurrentConfiguration[currentRobotModel][port];
		// TODO: It must depend on port, port must return its variable
		QString const variableName = "Sensor" + QString::number(index + 1);
		mWatcher->addTrackingObject(index, variableName, device.friendlyName());
		++index;
	}
}
