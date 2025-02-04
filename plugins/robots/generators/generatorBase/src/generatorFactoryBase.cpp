/* Copyright 2007-2015 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "generatorBase/generatorFactoryBase.h"
#include "generatorBase/generatorCustomizer.h"

#include <kitBase/robotModel/robotModelUtils.h>

#include "simpleGenerators/nullGenerator.h"
#include "simpleGenerators/commentElementGenerator.h"
#include "simpleGenerators/ifElementGenerator.h"
#include "simpleGenerators/infiniteLoopGenerator.h"
#include "simpleGenerators/forLoopGenerator.h"
#include "simpleGenerators/whileLoopGenerator.h"
#include "simpleGenerators/forkCallGenerator.h"
#include "simpleGenerators/joinGenerator.h"
#include "simpleGenerators/killThreadGenerator.h"
#include "simpleGenerators/switchGenerator.h"
#include "simpleGenerators/functionElementGenerator.h"
#include "simpleGenerators/enginesGenerator.h"
#include "simpleGenerators/enginesStopGenerator.h"
#include "simpleGenerators/timerGenerator.h"
#include "simpleGenerators/beepGenerator.h"
#include "simpleGenerators/playToneGenerator.h"
#include "simpleGenerators/finalNodeGenerator.h"
#include "simpleGenerators/nullificationEncoderGenerator.h"
#include "simpleGenerators/clearScreenBlockGenerator.h"
#include "simpleGenerators/printTextBlockGenerator.h"
#include "simpleGenerators/waitForColorIntensityBlockGenerator.h"
#include "simpleGenerators/waitForLightBlockGenerator.h"
#include "simpleGenerators/waitForSonarBlockGenerator.h"
#include "simpleGenerators/waitForEncoderBlockGenerator.h"
#include "simpleGenerators/waitForTouchSensorBlockGenerator.h"
#include "simpleGenerators/waitForSoundBlockGenerator.h"
#include "simpleGenerators/waitForGyroscopeBlockGenerator.h"
#include "simpleGenerators/waitForAccelerometerBlockGenerator.h"
#include "simpleGenerators/initialNodeGenerator.h"
#include "simpleGenerators/subprogramsSimpleGenerator.h"
#include "simpleGenerators/breakGenerator.h"
#include "simpleGenerators/continueGenerator.h"
#include "simpleGenerators/labelGenerator.h"
#include "simpleGenerators/gotoSimpleGenerator.h"
#include "simpleGenerators/variableInitGenerator.h"
#include "simpleGenerators/randomInitGenerator.h"
#include "simpleGenerators/sendMessageThreadsGenerator.h"
#include "simpleGenerators/receiveMessageThreadsGenerator.h"
#include "simpleGenerators/getButtonCodeGenerator.h"
#include "generatorBase/simpleGenerators/waitForButtonGenerator.h"

#include "converters/nameNormalizerConverter.h"
#include "converters/inequalitySignConverter.h"
#include "converters/breakModeConverter.h"
#include "converters/portNameConverter.h"
#include "converters/enginePortsConverter.h"
#include "converters/typeConverter.h"
#include "converters/reservedVariablesConverter.h"
#include "converters/functionBlockConverter.h"
#include "converters/intPropertyConverter.h"
#include "converters/floatPropertyConverter.h"
#include "converters/boolPropertyConverter.h"
#include "converters/switchConditionsMerger.h"
#include "converters/stringPropertyConverter.h"
#include "converters/dynamicPropertiesConverter.h"

#include "generatorBase/parts/variables.h"
#include "generatorBase/parts/subprograms.h"
#include "generatorBase/parts/threads.h"
#include "generatorBase/parts/engines.h"
#include "generatorBase/parts/sensors.h"
#include "generatorBase/parts/functions.h"

#include "generatorBase/lua/luaProcessor.h"

using namespace generatorBase;
using namespace qReal;
using namespace simple;
using namespace kitBase::robotModel;

#define RETURN_GENERATOR_PTR(type, arguments) \
	{ \
		return new type arguments; \
	}

GeneratorFactoryBase::GeneratorFactoryBase(const qrRepo::RepoApi &repo
		, ErrorReporterInterface &errorReporter
		, const RobotModelManagerInterface &robotModelManager
		, lua::LuaProcessor &luaProcessor)
	: mRepo(repo)
	, mErrorReporter(errorReporter)
	, mRobotModelManager(robotModelManager)
	, mLuaTranslator(luaProcessor)
{
}

GeneratorFactoryBase::~GeneratorFactoryBase() = default;

void GeneratorFactoryBase::initialize()
{
	mLuaTranslator.setPathsToTemplates(pathsToTemplates());
	initVariables();
	initSubprograms();
	mThreads.reset(new parts::Threads(pathsToTemplates()));
	initEngines();
	initSensors();
	initFunctions();
	initDeviceVariables();
}

void GeneratorFactoryBase::setMainDiagramId(const Id &diagramId)
{
	mDiagram = diagramId;
	mSensors->reinitDevices(currentConfiguration());
}

void GeneratorFactoryBase::initVariables()
{
	mVariables.reset(new parts::Variables(pathsToTemplates(), mRobotModelManager.model(), mLuaTranslator.toolbox()));
}

void GeneratorFactoryBase::initSubprograms()
{
	mSubprograms.reset(new parts::Subprograms(mRepo, mErrorReporter
			, pathsToTemplates(), mLuaTranslator.toolbox(), nameNormalizerConverter(), typeConverter()));
}

void GeneratorFactoryBase::initEngines()
{
	mEngines.reset(new parts::Engines(pathsToTemplates(), portNameConverter(), enginesConverter()));
}

void GeneratorFactoryBase::initSensors()
{
	mSensors.reset(new parts::Sensors(pathsToTemplates(), portNameConverter()));
}

void GeneratorFactoryBase::initFunctions()
{
	mFunctions.reset(new parts::Functions(pathsToTemplates()));
}

void GeneratorFactoryBase::initDeviceVariables()
{
	mDeviceVariables.reset(new parts::DeviceVariables());
}

QList<parts::InitTerminateCodeGenerator *> GeneratorFactoryBase::initTerminateGenerators()
{
	return QList<parts::InitTerminateCodeGenerator *>()
			<< engines() << sensors() << functions();
}

parts::Variables *GeneratorFactoryBase::variablesInfo() const
{
	return mVariables.get();
}

parts::Variables *GeneratorFactoryBase::variables()
{
	return mVariables.get();
}

parts::Subprograms *GeneratorFactoryBase::subprograms()
{
	return mSubprograms.get();
}

parts::Threads &GeneratorFactoryBase::threads()
{
	return *mThreads;
}

parts::Engines *GeneratorFactoryBase::engines()
{
	return mEngines.get();
}

parts::Sensors *GeneratorFactoryBase::sensors()
{
	return mSensors.get();
}

parts::Functions *GeneratorFactoryBase::functions()
{
	return mFunctions.get();
}

parts::DeviceVariables *GeneratorFactoryBase::deviceVariables() const
{
	return mDeviceVariables.get();
}

simple::AbstractSimpleGenerator *GeneratorFactoryBase::ifGenerator(const Id &id
		, GeneratorCustomizer &customizer
		, bool elseIsEmpty
		, bool needInverting)
RETURN_GENERATOR_PTR(IfElementGenerator, (mRepo, customizer, id, elseIsEmpty, needInverting, this))


simple::AbstractSimpleGenerator *GeneratorFactoryBase::infiniteLoopGenerator(const Id &id
		, GeneratorCustomizer &customizer)
RETURN_GENERATOR_PTR(InfiniteLoopGenerator, (mRepo, customizer, id, this))


simple::AbstractSimpleGenerator *GeneratorFactoryBase::whileLoopGenerator(const Id &id
		, GeneratorCustomizer &customizer
		, bool doWhileForm
		, bool needInverting)
RETURN_GENERATOR_PTR(WhileLoopGenerator, (mRepo, customizer, id, doWhileForm, needInverting, this))


simple::AbstractSimpleGenerator *GeneratorFactoryBase::forLoopGenerator(const Id &id
		, GeneratorCustomizer &customizer)
RETURN_GENERATOR_PTR(ForLoopGenerator, (++mLoopGeneratorIndex, mRepo, customizer, id, this))

AbstractSimpleGenerator *GeneratorFactoryBase::switchHeadGenerator(const Id &id
		, GeneratorCustomizer &customizer, const QStringList &values, bool generateIfs)
RETURN_GENERATOR_PTR(SwitchGenerator, (mRepo, customizer, id, "head", values, generateIfs, this))

AbstractSimpleGenerator *GeneratorFactoryBase::switchMiddleGenerator(const Id &id
		, GeneratorCustomizer &customizer, const QStringList &values, bool generateIfs)
RETURN_GENERATOR_PTR(SwitchGenerator, (mRepo, customizer, id, "middle", values, generateIfs, this))

AbstractSimpleGenerator *GeneratorFactoryBase::switchDefaultGenerator(const Id &id
		, GeneratorCustomizer &customizer, bool generateIfs)
RETURN_GENERATOR_PTR(SwitchGenerator, (mRepo, customizer, id, "default", {}, generateIfs, this))

AbstractSimpleGenerator *GeneratorFactoryBase::forkCallGenerator(const Id &id
		, GeneratorCustomizer &customizer, const QMap<Id, QString> &threads)
RETURN_GENERATOR_PTR(ForkCallGenerator, (mRepo, customizer, id, threads, this))

AbstractSimpleGenerator *GeneratorFactoryBase::joinGenerator(const Id &id, GeneratorCustomizer &customizer
		, const QStringList &joinedThreads, const QString &mainThreadId)
RETURN_GENERATOR_PTR(JoinGenerator, (mRepo, customizer, id, joinedThreads, mainThreadId, this))

AbstractSimpleGenerator *GeneratorFactoryBase::simpleGenerator(const qReal::Id &id
		, GeneratorCustomizer &customizer)
{
	const QString elementType = id.element();

	if (elementType == "CommentBlock") {
		return new CommentElementGenerator(mRepo, customizer, id, this);
	} else if (elementType == "Function") {
		return new FunctionElementGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("EnginesForward") || elementType.contains("EnginesBackward")) {
		return new EnginesGenerator(mRepo, customizer, id, elementType, this);
	} else if (elementType.contains("EnginesStop")) {
		return new EnginesStopGenerator(mRepo, customizer, id, this);
	} else if (elementType == "Timer") {
		return new TimerGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("Beep")) {
		return new BeepGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("PlayTone")) {
		return new PlayToneGenerator(mRepo, customizer, id, this);
	} else if (elementType == "InitialNode") {
		return new InitialNodeGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("ClearEncoder")) {
		return new NullificationEncoderGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForButton")) {
		return new WaitForButtonGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForColorIntensity")) {
		return new WaitForColorIntensityBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForLight")) {
		return new WaitForLightBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForTouchSensor")) {
		return new WaitForTouchSensorBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForSonarDistance")) {
		return new WaitForSonarBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForEncoder")) {
		return new WaitForEncoderBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForSound")) {
		return new WaitForSoundBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForGyroscope")) {
		return new WaitForGyroscopeBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("WaitForAccelerometer")) {
		return new WaitForAccelerometerBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("ClearScreen")) {
		return new ClearScreenBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType.contains("PrintText")) {
		return new PrintTextBlockGenerator(mRepo, customizer, id, this);
	} else if (elementType == "Subprogram") {
		return new SubprogramsSimpleGenerator(mRepo, customizer, id, this);
	} else if (elementType == "VariableInit") {
		return new VariableInitGenerator(mRepo, customizer, id, this);
	} else if (elementType == "Randomizer") {
		return new RandomInitGenerator(mRepo, customizer, id, this);
	} else if (elementType == "SendMessageThreads") {
		return new SendMessageThreadsGenerator(mRepo, customizer, id, this);
	} else if (elementType == "ReceiveMessageThreads") {
		return new ReceiveMessageThreadsGenerator(mRepo, customizer, id, this);
	} else if (elementType == "KillThread") {
		return new KillThreadGenerator(mRepo, customizer, id, this);
	} else if (elementType == "GetButtonCode") {
		return new GetButtonCodeGenerator(mRepo, customizer, id, this);
	}

	return new NullGenerator(mRepo, customizer, id, this);
}

simple::AbstractSimpleGenerator *GeneratorFactoryBase::breakGenerator(const Id &id
		, GeneratorCustomizer &customizer)
RETURN_GENERATOR_PTR(BreakGenerator, (mRepo, customizer, id, this))

simple::AbstractSimpleGenerator *GeneratorFactoryBase::continueGenerator(const Id &id
		, GeneratorCustomizer &customizer)
RETURN_GENERATOR_PTR(ContinueGenerator, (mRepo, customizer, id, this))

AbstractSimpleGenerator *GeneratorFactoryBase::labelGenerator(const qReal::Id &id
		, GeneratorCustomizer &customizer)
RETURN_GENERATOR_PTR(LabelGenerator, (mRepo, customizer, id, this))

AbstractSimpleGenerator *GeneratorFactoryBase::gotoSimpleGenerator(const qReal::Id &id
		, GeneratorCustomizer &customizer)
RETURN_GENERATOR_PTR(GotoSimpleGenerator, (mRepo, customizer, id, this))

AbstractSimpleGenerator *GeneratorFactoryBase::finalNodeGenerator(const qReal::Id &id
		, GeneratorCustomizer &customizer, bool fromMainGenerator)
RETURN_GENERATOR_PTR(FinalNodeGenerator, (mRepo, customizer, id, fromMainGenerator, this))

// Converters can be instantiated without taking ownership because binders do this

Binding::ConverterInterface *GeneratorFactoryBase::intPropertyConverter(const Id &id, const QString &property) const
RETURN_GENERATOR_PTR(converters::IntPropertyConverter, (pathsToTemplates(), mLuaTranslator, id
			, property, reservedVariableNameConverter()))

Binding::ConverterInterface *GeneratorFactoryBase::floatPropertyConverter(const Id &id, const QString &property) const
RETURN_GENERATOR_PTR(converters::FloatPropertyConverter,
							(mLuaTranslator, id, property, reservedVariableNameConverter()))

Binding::ConverterInterface *GeneratorFactoryBase::boolPropertyConverter(const Id &id
		, const QString &property, bool needInverting) const
RETURN_GENERATOR_PTR(converters::BoolPropertyConverter, (pathsToTemplates(), mLuaTranslator
			, id, property, reservedVariableNameConverter(), needInverting))

Binding::ConverterInterface *GeneratorFactoryBase::stringPropertyConverter(const qReal::Id &block
		, const QString &property) const
RETURN_GENERATOR_PTR(converters::StringPropertyConverter,
							(mLuaTranslator, block, property, reservedVariableNameConverter()))

Binding::ConverterInterface *GeneratorFactoryBase::reservedVariableNameConverter() const
RETURN_GENERATOR_PTR(converters::ReservedVariablesConverter,(pathsToTemplates()
			, mErrorReporter
			, mRobotModelManager.model()
			, currentConfiguration()
			, portNameConverter()
			, *deviceVariables()))

Binding::ConverterInterface *GeneratorFactoryBase::nameNormalizerConverter() const
RETURN_GENERATOR_PTR(converters::NameNormalizerConverter, ())

Binding::ConverterInterface *GeneratorFactoryBase::dynamicPropertiesConverter(const qReal::Id &block) const
RETURN_GENERATOR_PTR(converters::DynamicPropertiesConverter,
							(mLuaTranslator, block, pathsToTemplates(), reservedVariableNameConverter()))

Binding::ConverterInterface *GeneratorFactoryBase::functionBlockConverter(const qReal::Id &block
		, const QString &property) const
RETURN_GENERATOR_PTR(converters::FunctionBlockConverter,
							(mLuaTranslator, block, property, reservedVariableNameConverter()))

Binding::ConverterInterface *GeneratorFactoryBase::inequalitySignConverter() const
RETURN_GENERATOR_PTR(converters::InequalitySignConverter, (pathsToTemplates()))

Binding::MultiConverterInterface *GeneratorFactoryBase::enginesConverter() const
RETURN_GENERATOR_PTR(converters::EnginePortsConverter, (portNameConverter()))

Binding::ConverterInterface *GeneratorFactoryBase::portNameConverter() const
RETURN_GENERATOR_PTR(converters::PortNameConverter, (pathsToTemplates()
			, mRobotModelManager.model().availablePorts()
			, mErrorReporter))

Binding::ConverterInterface *GeneratorFactoryBase::breakModeConverter() const
RETURN_GENERATOR_PTR(converters::BreakModeConverter, (pathsToTemplates()))

Binding::ConverterInterface *GeneratorFactoryBase::typeConverter() const
RETURN_GENERATOR_PTR(converters::TypeConverter, (pathsToTemplates()))

Binding::ConverterInterface *GeneratorFactoryBase::switchConditionsMerger(const QStringList &values
		, bool generateIf) const
RETURN_GENERATOR_PTR(converters::SwitchConditionsMerger,
							(pathsToTemplates(), reservedVariableNameConverter(), values, generateIf))

QString GeneratorFactoryBase::initCode()
{
	QStringList result;
	QList<parts::InitTerminateCodeGenerator *> const generators(initTerminateGenerators());
	for (parts::InitTerminateCodeGenerator *generator : generators) {
		result << generator->initCode();
	}

	return result.join('\n');
}

QString GeneratorFactoryBase::terminateCode()
{
	QStringList result;
	QList<parts::InitTerminateCodeGenerator *> const generators(initTerminateGenerators());
	for (parts::InitTerminateCodeGenerator *generator : generators) {
		result << generator->terminateCode();
	}

	return result.join('\n');
}

QString GeneratorFactoryBase::isrHooksCode()
{
	QStringList result;
	QList<parts::InitTerminateCodeGenerator *> const generators(initTerminateGenerators());
	for (parts::InitTerminateCodeGenerator *generator : generators) {
		result << generator->isrHooksCode();
	}

	return result.join('\n');
}

QMap<PortInfo, DeviceInfo> GeneratorFactoryBase::currentConfiguration() const
{
	auto worldModel = mRepo.metaInformation("worldModel").toString();
	const auto id = mRobotModelManager.model().robotId();
	auto robotConfig = RobotModelUtils::deserializeFromWorldModel(worldModel)[id];
	// At the moment we have sensors configuration from widget-configurer. We must also add here non-configurable
	// by user devices (like encoders, displays and so on).
	for (const PortInfo &port : mRobotModelManager.model().availablePorts()) {
		if (!mRobotModelManager.model().configurablePorts().contains(port)
				&& mRobotModelManager.model().allowedDevices(port).count() == 1
				&& port.direction() == input) {
			robotConfig[port] = mRobotModelManager.model().allowedDevices(port)[0];
		}
	}

	return robotConfig;
}
