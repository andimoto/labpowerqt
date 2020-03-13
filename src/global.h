// This file is part of labpowerqt, a Gui application to control programmable
// lab power supplies.
// Copyright © 2015, 2016 Christian Rapp <0x2a at posteo dot org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef GLOBAL
#define GLOBAL

#include <QLayout>
#include <QLayoutItem>
#include <QWidget>

// Suppress attribute unused warnings on gcc
#ifdef __GNUC__
#define ATTR_UNUSED __attribute__((unused))
#else
#define ATTR_UNUSED
#endif

/**
 * @brief Namespace for globally used constants
 */
namespace global_constants
{
enum class LPQ_PROTOCOL { KORADV2 = 0, MANSON_HCS3X };
enum class LPQ_CHANNEL { CHANNEL1 = 1, CHANNEL2, CHANNEL3, CHANNEL4 };
enum class LPQ_MODE { CONSTANT_CURRENT = 0, CONSTANT_VOLTAGE };
enum class LPQ_TRACKING { INDEPENDENT = 0, SERIES, PARALELL };
enum class LPQ_DATATYPE {
    SETVOLTAGE = 0,
    VOLTAGE,
    SETCURRENT,
    CURRENT,
    WATTAGE
};
enum class LPQ_CONTROL {
    CONNECT = 0,
    OCP,
    OVP,
    OTP,
    OUTPUT,
    SOUND,
    LOCK,
    DEVICEID
};

const char *const ORANGECOLOR = "#FF7E00";
const char *const GREENCOLOR = "#7BCF06";
}

namespace global_utilities
{
/**
 * @brief A method that recursively deletes all Widgets and Items inside a QLayout
 */
inline void clearLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            delete widget;

        // recursive if the layout has child layouts.
        if (QLayout *childLayout = item->layout())
            global_utilities::clearLayout(childLayout);
        delete item;
    }
};
}

#endif  // GLOBAL
