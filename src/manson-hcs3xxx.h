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

#ifndef MANSON_HCS3XXX_H
#define MANSON_HCS3XXX_H

#include <QObject>
#include <QString>

#include "config.h"
#include "global.h"
#include "log_instance.h"
#include "powersupplyscpi.h"

namespace MansonHCS3x_constants
{
namespace powcon = PowerSupplySCPI_constants;
/**
 * @brief Map internal commands to SCPI syntax
 *
 * @details
 * Every derives class of PowerSupplySCPI has to provide this map
 */
const std::map<int, QString> SERIALCOMMANDMAP = {
    {powcon::SETCURRENTSET, "ISET%1:%2"}, /**< Set current */
    {powcon::GETCURRENTSET, "ISET%1?"},   /**< Get current that has been set */
    {powcon::SETVOLTAGESET, "VSET%1:%2"}, /**< Set voltage */
    {powcon::GETVOLTAGESET, "VSET%1?"},   /**< Get voltage that has been set */
    {powcon::GETCURRENT, "IOUT%1?"},      /**< Get actual current */
    {powcon::GETVOLTAGE, "VOUT%1?"},      /**< Get actual Voltage */
    {powcon::SETCHANNELTRACKING, "TRACK%1"}, /**< Selects the operation mode:
                                            independent, trackingseries, or
                                            tracking parallel. */
    {powcon::SETBEEP, "BEEP%1"},             // turn beep on or off
    {powcon::SETOUT, "OUT%1"},               // turn output on or off
    {powcon::GETSTATUS, "STATUS?"},          // request status
    {powcon::GETIDN, "*IDN?"},               // get device identification string
    {powcon::GETSAVEDSETTINGS, "RCL%1"},     // set device to memorized settings
    {powcon::SAVESETTINGS, "SAV%1"},  // save current settings on memory position
    {powcon::SETOCP, "OCP%1"},        // switch over current protection
    {powcon::SETOVP, "OVP%1"},        // switch over voltage protection
    {powcon::GETOVP,
     "OVP%1?"},  // dummy command because firmware does not support this
    {powcon::GETOCP,
     "OCP%1?"},  // dummy command because firmware does not support this
    {powcon::SETDUMMY, "DUMMY"},  // just some dummy command
};
const std::map<int, int> SERIALCOMMANDBUFLENGTH = {
    {powcon::GETVOLTAGESET, 5}, /**< Get voltage that has been set */
    {powcon::GETVOLTAGE, 5},    /**< Get actual Voltage */
    {powcon::GETCURRENTSET, 5},
    {powcon::GETCURRENT, 5},  /**< Get actual current */
    {powcon::GETSTATUS, 50},  // request status
    {powcon::GETIDN, 50}      // get device identification string
};
}

/**
 * @brief The MansonHCS3x class is an Implementation of the SCPI Interface for Manson/Voltcraft Power Supplies.
 *
 * @details
 *
 */
class MansonHCS3x : public PowerSupplySCPI
{
    Q_OBJECT

public:
    /**
     * @brief KoradSPIC
     * @param serialPortName
     * @throw std::runtime_error when Serial Port could not be opened
     */
	MansonHCS3x(QString serialPortName, QByteArray deviceHash, int noOfChannels,
              int voltageAccuracy, int currentAccuracy,
              QSerialPort::BaudRate brate, QSerialPort::FlowControl flowctl,
              QSerialPort::DataBits dbits, QSerialPort::Parity parity,
              QSerialPort::StopBits sbits, int portTimeOut);
    ~MansonHCS3x();

    // LabPowerSupply Interface
    void getIdentification();
    void getStatus();
    void changeChannel(int channel);
    void setVoltage(int channel, double value);
    void getVoltage(int channel);
    void getActualVoltage(int channel);
    void setCurrent(int channel, double value);
    void getCurrent(int channel);
    void getActualCurrent(int channel);
    void setOCP(bool status);
    void setOVP(bool status);
    /**
     * @brief Not provided by Protocol
     * @param status
     */
    void setOTP(bool status);
    /**
     * @brief Not provided by Protocol
     * @param status
     */
    void setLocked(bool status);
    void setBeep(bool status);
    void setTracking(global_constants::LPQ_TRACKING trMode);
    void setOutput(int channel, bool status);

signals:

private:
    // LabPowerSupply Interface
    QByteArray prepareCommandByteArray(
        const std::shared_ptr<SerialCommand> &com);
    std::vector<std::shared_ptr<SerialCommand>> prepareStatusCommands();
    void processCommands(const std::shared_ptr<PowerSupplyStatus> &status,
                         const std::shared_ptr<SerialCommand> &com);
    void updateNewPStatus(const std::shared_ptr<PowerSupplyStatus> &status);
    void calculateWattage(const std::shared_ptr<PowerSupplyStatus> &status);

    bool ocp;
    bool ovp;

private slots:
    void deviceInitialization();
};

#endif  // MANSON_HCS3XXX_H
