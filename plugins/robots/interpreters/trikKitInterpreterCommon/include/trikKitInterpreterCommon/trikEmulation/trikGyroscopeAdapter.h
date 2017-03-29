#pragma once

#include <QtCore/QTimer>
#include <QtGui/QQuaternion>

#include <trikControl/gyroSensorInterface.h>
#include <trikKitInterpreterCommon/robotModel/twoD/trikTwoDRobotModel.h>
#include <kitBase/robotModel/robotParts/gyroscopeSensor.h>

namespace kitBase {
namespace robotModel {
namespace robotParts {
class GyroscopeSensor;
}
}
}

class TrikGyroscopeAdapter : public trikControl::GyroSensorInterface {

	Q_OBJECT

public:
	TrikGyroscopeAdapter(kitBase::robotModel::robotParts::GyroscopeSensor *gyro
		, const QSharedPointer<trik::robotModel::twoD::TrikTwoDRobotModel> &model);

	virtual Status status() const override { return Status::ready; }

public slots:
	virtual QVector<int> read() const override;
	virtual void calibrate(int msec) override;
	virtual bool isCalibrated() const override;
	virtual QVector<int> readRawData() const override;

private slots:
	void countTilt(const QVector<int> &oldFormat);

private:
	qreal degreeToMilidegree(qreal value);
	int convertToTrikRuntimeTime(quint64 time) const;

	bool mTimeInited = false;

	kitBase::robotModel::robotParts::GyroscopeSensor *mGyro;
	QSharedPointer<trik::robotModel::twoD::TrikTwoDRobotModel> mModel;
	QVector<int> mResult;
	QQuaternion mQuaternion;
	quint64 mStartTime;
	quint64 mLastUpdateTimeStamp;
};
