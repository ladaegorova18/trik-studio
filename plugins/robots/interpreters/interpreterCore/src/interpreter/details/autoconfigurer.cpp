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

#include "interpreterCore/interpreter/details/autoconfigurer.h"

#include <qrkernel/exception/exception.h>

#include <kitBase/blocksBase/robotsBlock.h>
#include <kitBase/robotModel/robotModelInterface.h>

using namespace interpreterCore::interpreter::details;
using namespace qReal;
using namespace kitBase::robotModel;

Autoconfigurer::Autoconfigurer(const GraphicalModelAssistInterface &graphicalModelApi
		, const LogicalModelAssistInterface &logicalModelApi
		, BlocksTable &blocksTable
		, qReal::ErrorReporterInterface &errorReporter)
	: mGraphicalModelApi(graphicalModelApi)
	, mLogicalModelApi(logicalModelApi)
	, mBlocksTable(blocksTable)
	, mErrorReporter(errorReporter)
{
}

bool Autoconfigurer::configure(QList<qReal::Id> const &diagrams, const QString &robotId)
{
	for (const Id &diagram : diagrams) {
		const IdList children = mGraphicalModelApi.graphicalRepoApi().children(diagram);

		for (const Id &child : children) {
			kitBase::blocksBase::RobotsBlock * const block
					= dynamic_cast<kitBase::blocksBase::RobotsBlock *>(mBlocksTable.block(child));
			if (!block) {
				continue;
			}

			QMap<PortInfo, DeviceInfo> const usedDevices = block->usedDevices();
			for (const PortInfo &port : usedDevices.keys()) {
				const DeviceInfo device = usedDevices[port];
				const DeviceInfo existingDevice = currentConfiguration(robotId, port);
				if (existingDevice.isA(device)) {
					// Do nothing
				} else if (!mLogicalModelApi.mutableLogicalRepoApi()
						.metaInformation("twoDModelSensorsReadOnly").toBool() && existingDevice.isNull()) {
					mErrorReporter.addInformation(QObject::tr("%2 has been auto configured to "\
							"port %1").arg(port.name(), device.friendlyName()), child);
					deviceConfigurationChanged(robotId, port, device, Reason::automaticConfiguration);
				} else {
					mErrorReporter.addError(QObject::tr("Sensor on port %1 does not correspond to blocks "\
							"on the diagram.").arg(port.name()), child);
					return false;
				}
			}
		}
	}

	return true;
}
