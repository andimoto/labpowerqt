// labpowerqt is a Gui application to control programmable lab power supplies
// Copyright © 2015 Christian Rapp <0x2a at posteo dot org>

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

#ifndef YAXISHELPER_H
#define YAXISHELPER_H

#include <QMap>

#include <vector>
#include <algorithm>

#include "qcustomplot.h"

#include "global.h"

struct YAxisBounds {
    double voltageUpper;
    double voltageLower;
    double currentUpper;
    double currentLower;
    double wattageUpper;
    double wattageLower;

    YAxisBounds()
    {
        voltageUpper = 0;
        voltageLower = 0;
        currentUpper = 0;
        currentLower = 0;
        wattageUpper = 0;
        wattageLower = 0;
    }
};

class YAxisHelper
{
public:
    YAxisHelper();

    YAxisBounds getyAxisBounds(const QCPRange &currentXRange, QCustomPlot *plot,
                               int noChannels);

private:
    std::pair<double, double> lowHighVectorValue(std::vector<double> values);
};

#endif // YAXISHELPER_H
