// labpowerqt is a Gui application to control programmable lab power supplies
// Copyright © 2015, 2016 Christian Rapp <0x2a at posteo dot org>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "manson-hcs3xxx.h"

namespace globcon = global_constants;
namespace powcon = PowerSupplySCPI_constants;
namespace mansoncon = MansonHCS3x_constants;
namespace statuscon = PowerSupplyStatus_constants;

MansonHCS3x::MansonHCS3x(QString serialPortName, QByteArray deviceHash,
                     int noOfChannels, int voltageAccuracy, int currentAccuracy,
                     QSerialPort::BaudRate brate,
                     QSerialPort::FlowControl flowctl,
                     QSerialPort::DataBits dbits, QSerialPort::Parity parity,
                     QSerialPort::StopBits sbits, int portTimeOut)
    : PowerSupplySCPI(std::move(serialPortName), std::move(deviceHash),
                      noOfChannels, voltageAccuracy, currentAccuracy, brate,
                      flowctl, dbits, parity, sbits, portTimeOut)
{
    this->canCalculateWattage = true;

    this->statusCommands = {powcon::GETSTATUS, powcon::GETCURRENT,
                            powcon::GETVOLTAGE};

    QObject::connect(this, &MansonHCS3x::deviceOpen, this,
                     &MansonHCS3x::deviceInitialization);
}

MansonHCS3x::~MansonHCS3x()
{
    // We must make sure our thread has finished PowerSupplySCPI::readWriteData
    // before an Object of this class gets destroyed. The Problem is every
    // derived class must do this.
    // TODO: Document all special implementations needed by a derived class of
    // PowerSupplySCPI
    QMutexLocker qlock(&this->qserialPortGuard);
}

void MansonHCS3x::getIdentification()
{
    // parameters two and three are irrelevant here.
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::GETIDN), 0,
                        QVariant(0), true, 50);
}

void MansonHCS3x::getStatus()
{
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::GETSTATUS), 0,
                        QVariant(0), true, 50);
}

void MansonHCS3x::changeChannel(ATTR_UNUSED int channel) {}
void MansonHCS3x::setVoltage(int channel, double value)
{
    this->powStatus->setVoltageSet(std::make_pair(channel, value));
    this->serQueue.push(
        static_cast<int>(powcon::COMMANDS::SETVOLTAGESET), channel,
        QVariant(QString::number(value, 'f', this->voltageAccuracy)));
}

void MansonHCS3x::getVoltage(int channel)
{
    this->serQueue.push(
        static_cast<int>(powcon::COMMANDS::GETVOLTAGESET), channel, QVariant(),
        true,
		mansoncon::SERIALCOMMANDBUFLENGTH.at(powcon::COMMANDS::GETVOLTAGESET));
}

void MansonHCS3x::getActualVoltage(int channel)
{
    this->serQueue.push(
        static_cast<int>(powcon::COMMANDS::GETVOLTAGE), channel, QVariant(),
        true, mansoncon::SERIALCOMMANDBUFLENGTH.at(powcon::COMMANDS::GETVOLTAGE));
}

void MansonHCS3x::setCurrent(int channel, double value)
{
    this->powStatus->setCurrentSet(std::make_pair(channel, value));
    this->serQueue.push(
        static_cast<int>(powcon::COMMANDS::SETCURRENTSET), channel,
        QVariant(QString::number(value, 'f', this->currentAccuracy)));
}

void MansonHCS3x::getCurrent(int channel)
{
    this->serQueue.push(
        static_cast<int>(powcon::COMMANDS::GETCURRENTSET), channel, QVariant(),
        true,
		mansoncon::SERIALCOMMANDBUFLENGTH.at(powcon::COMMANDS::GETCURRENTSET));
}

void MansonHCS3x::getActualCurrent(int channel)
{
    this->serQueue.push(
        static_cast<int>(powcon::COMMANDS::GETCURRENT), channel, QVariant(),
        true, mansoncon::SERIALCOMMANDBUFLENGTH.at(powcon::COMMANDS::GETCURRENT));
}

void MansonHCS3x::setOCP(bool status)
{
    this->powStatus->setOcp(status);
    QVariant val = 0;
    if (status) {
        val = 1;
    }
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::SETOCP), 0, val);
}

void MansonHCS3x::setOVP(bool status)
{
    this->powStatus->setOvp(status);
    QVariant val = 0;
    if (status) {
        val = 1;
    }
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::SETOVP), 0, val);
}

void MansonHCS3x::setOTP(ATTR_UNUSED bool status) {}
void MansonHCS3x::setLocked(ATTR_UNUSED bool status) {}
void MansonHCS3x::setBeep(bool status)
{
    QVariant val = 0;
    if (status) {
        val = 1;
    }
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::SETBEEP), 0, val);
}

void MansonHCS3x::setTracking(ATTR_UNUSED globcon::LPQ_TRACKING trMode)
{
    // TODO: Implement Tracking in Korad SCPI Class.
}

void MansonHCS3x::setOutput(ATTR_UNUSED int channel, bool status)
{
    QVariant val = 0;
    if (status) {
        val = 1;
    }
    // sending 0 as Korad SCPI interface does not support different channels
    this->serQueue.push(static_cast<int>(powcon::COMMANDS::SETOUT), 0, val);
}

void MansonHCS3x::processCommands(const std::shared_ptr<PowerSupplyStatus> &status,
                                const std::shared_ptr<SerialCommand> &com)
{
    LogInstance::get_instance().eal_debug("Processing command " +
                                          std::to_string(com->getCommand()));
    LogInstance::get_instance().eal_debug(
        "Command value" + com->getValue().toString().toStdString());
    if (com->getCommand() == powcon::COMMANDS::GETSTATUS) {
        /*
         * Decoding the Korad Status Byte is pretty simple.
         * MSB -> LSB
         * 7   not defined
         * 6   Output
         * 5   Lock
         * 4   Beep
         * 2,3 Channel Tracking Mode
         * 1   CH2 CC|CV mode
         * 0   CH1 CC|CV mode
         */
        QByteArray val = com->getValue().toByteArray();
        LogInstance::get_instance().eal_debug(
            "Manson Status byte: " + std::string(val.constData(), val.length()));
        // Unfortunately Korad SCPI does not seem to be able to determine
        // between different channels regarding output setting.
        if (val[0] & (1 << 6)) {
            for (int i = 1; i <= this->noOfChannels; i++) {
                status->setChannelOutput(std::make_pair(i, true));
            }
        } else {
            for (int i = 1; i <= this->noOfChannels; i++) {
                status->setChannelOutput(std::make_pair(i, false));
            }
        }
        if (val[0] & (1 << 5)) {
            status->setLocked(true);
        }
        if (val[0] & (1 << 4)) {
            status->setBeeper(true);
        }

        // TODO: Add check for tracking mode

        if (val[0] & (1 << 1)) {
            status->setChannelMode(
                std::make_pair(2, globcon::LPQ_MODE::CONSTANT_VOLTAGE));
        } else {
            status->setChannelMode(
                std::make_pair(2, globcon::LPQ_MODE::CONSTANT_CURRENT));
        }
        if (val[0] & (1 << 0)) {
            status->setChannelMode(
                std::make_pair(1, globcon::LPQ_MODE::CONSTANT_VOLTAGE));
        } else {
            status->setChannelMode(
                std::make_pair(1, globcon::LPQ_MODE::CONSTANT_CURRENT));
        }
    }

    if (com->getCommand() == powcon::COMMANDS::GETIDN) {
        QString val = com->getValue().toString();
    }

    if (com->getCommand() == powcon::COMMANDS::GETCURRENTSET) {
        QString val = com->getValue().toString();

        // strangely current values seem to end with a "K". Therefor it is not
        // possible to get a double with toDouble() directly. This fimrware is
        // really buggy.
        if (val.endsWith("K", Qt::CaseSensitivity::CaseInsensitive)) {
            val = val.left(val.length() - 1);
            com->setValue(std::move(val));
        }
    }

    if (com->getCommand() == powcon::COMMANDS::GETCURRENT) {
        if (status)
            status->setCurrent(std::make_pair(com->getPowerSupplyChannel(),
                                              com->getValue().toDouble()));
    }

    if (com->getCommand() == powcon::COMMANDS::GETVOLTAGESET) {
    }

    if (com->getCommand() == powcon::COMMANDS::GETVOLTAGE) {
        if (status)
            status->setVoltage(std::make_pair(com->getPowerSupplyChannel(),
                                              com->getValue().toDouble()));
    }
}

void MansonHCS3x::updateNewPStatus(
    const std::shared_ptr<PowerSupplyStatus> &status)
{
    for (int i = 1; i <= this->noOfChannels; i++) {
        status->setVoltageSet(
            std::make_pair(i, this->powStatus->getVoltageSet(i)));
        status->setCurrentSet(
            std::make_pair(i, this->powStatus->getCurrentSet(i)));
    }

    status->setOvp(this->powStatus->getOvp());
    status->setOcp(this->powStatus->getOcp());
    status->setOtp(this->powStatus->getOtp());
}

void MansonHCS3x::calculateWattage(
    const std::shared_ptr<PowerSupplyStatus> &status)
{
    for (int i = 1; i <= this->noOfChannels; i++) {
        // P = U*I :)
        double wattValue = status->getVoltage(i) * status->getCurrent(i);
        statuscon::CHANNELVALUE watt = std::make_pair(i, wattValue);
        status->setWattage(watt);
    }
}

void MansonHCS3x::deviceInitialization()
{
    // The Korad firmware does not allow to query the status of OCP and OVP. So
    // we have to disable them both at the beginning. Otherwise our Application
    // could be in an undefined state.
    // Edit 11.12.15: Why are these two function calls commented?
    // this->setOCP(false);
    // this->setOVP(false);
}

QByteArray MansonHCS3x::prepareCommandByteArray(
    const std::shared_ptr<SerialCommand> &com)
{
    // First job create the command
    powcon::COMMANDS command = static_cast<powcon::COMMANDS>(com->getCommand());
    if (command == powcon::COMMANDS::GETOVP ||
        command == powcon::COMMANDS::GETOCP)
        return "";
    QString commandString = mansoncon::SERIALCOMMANDMAP.at(command);
    /*
     * We have four different command types.
     * 1. Command that does something with a channel
     * 2. Command that does something with a channel and a value
     * 3. Command that does something with a value
     * 4. Command that does something
     */
    if (commandString.indexOf("%") > 0) {
        if (com->getPowerSupplyChannel() == 0) {
            commandString = commandString.arg(com->getValue().toString());
        } else {
            com->getValue().isNull() == true
                ? commandString = commandString.arg(com->getPowerSupplyChannel())
                : commandString = commandString.arg(com->getPowerSupplyChannel())
                                      .arg(com->getValue().toString());
        }
    }

    LogInstance::get_instance().eal_debug("Command string: " +
                                          commandString.toStdString());

    QByteArray commandByte = commandString.toLocal8Bit();
    return commandByte;
}

std::vector<std::shared_ptr<SerialCommand>> MansonHCS3x::prepareStatusCommands()
{
    std::vector<std::shared_ptr<SerialCommand>> comVec;
    std::shared_ptr<SerialCommand> com;
    for (const auto &c : this->statusCommands) {
        if (c == powcon::COMMANDS::GETVOLTAGE ||
            c == powcon::COMMANDS::GETCURRENT) {
            for (int i = 1; i <= this->noOfChannels; i++) {
                com = std::make_shared<SerialCommand>(
                    static_cast<int>(c), i, QVariant(), true,
					mansoncon::SERIALCOMMANDBUFLENGTH.at(c));
            }
        } else {
            com = std::make_shared<SerialCommand>(
                static_cast<int>(c), 1, QVariant(), true,
				mansoncon::SERIALCOMMANDBUFLENGTH.at(c));
        }
        comVec.push_back(com);
    }
    return comVec;
}
