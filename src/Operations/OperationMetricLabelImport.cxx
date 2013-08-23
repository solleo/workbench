/*LICENSE_START*/
/*
 *  Copyright 1995-2002 Washington University School of Medicine
 *
 *  http://brainmap.wustl.edu
 *
 *  This file is part of CARET.
 *
 *  CARET is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  CARET is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CARET; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "OperationMetricLabelImport.h"
#include "OperationException.h"

#include "CaretLogger.h"
#include "FileInformation.h"
#include "GiftiLabel.h"
#include "GiftiLabelTable.h"
#include "LabelFile.h"
#include "MetricFile.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

using namespace caret;
using namespace std;

AString OperationMetricLabelImport::getCommandSwitch()
{
    return "-metric-label-import";
}

AString OperationMetricLabelImport::getShortDescription()
{
    return "IMPORT A GIFTI LABEL FILE FROM A METRIC FILE";
}

OperationParameters* OperationMetricLabelImport::getParameters()
{
    OperationParameters* ret = new OperationParameters();
    ret->addMetricParameter(1, "input", "the input metric file");
    
    ret->addStringParameter(2, "label-list-file", "text file containing the values and names for labels");
    
    ret->addLabelOutputParameter(3, "output", "the output gifti label file");
    
    ret->createOptionalParameter(4, "-discard-others", "set any values not mentioned in the label list to the ??? label");
    
    OptionalParameter* unlabeledOption = ret->createOptionalParameter(5, "-unlabeled-value", "set the value that will be interpreted as unlabeled");
    unlabeledOption->addIntegerParameter(1, "value", "the numeric value for unlabeled (default 0)");
    
    OptionalParameter* columnSelect = ret->createOptionalParameter(6, "-column", "select a single column to import");
    columnSelect->addStringParameter(1, "column", "the column number or name");
    
    ret->createOptionalParameter(7, "-drop-unused-labels", "remove any unused label values from the label table");
    
    ret->setHelpText(
        AString("This is where you set the help text.  DO NOT add the info about what the command line format is, ") +
        "and do not give the command switch, short description, or the short descriptions of parameters.  Do not indent, " +
        "add newlines, or format the text in any way other than to separate paragraphs within the help text prose."
    );
    return ret;
}

void OperationMetricLabelImport::useParameters(OperationParameters* myParams, ProgressObject* myProgObj)
{
    LevelProgress myProgress(myProgObj);
    MetricFile* myMetric = myParams->getMetric(1);
    AString listfileName = myParams->getString(2);
    LabelFile* myLabelOut = myParams->getOutputLabel(3);
    bool discardOthers = myParams->getOptionalParameter(4)->m_present;
    int32_t unlabeledValue = 0;
    OptionalParameter* unlabeledOption = myParams->getOptionalParameter(5);
    if (unlabeledOption->m_present)
    {
        unlabeledValue = (int32_t)unlabeledOption->getInteger(1);
    }
    OptionalParameter* columnSelect = myParams->getOptionalParameter(6);
    int columnNum = -1;
    if (columnSelect->m_present)
    {
        columnNum = (int)myMetric->getMapIndexFromNameOrNumber(columnSelect->getString(1));
        if (columnNum < 0 || columnNum >= myMetric->getNumberOfMaps())
        {
            throw OperationException("invalid column specified");
        }
    }
    bool dropUnused = myParams->getOptionalParameter(7)->m_present;
    map<int32_t, int32_t> translate;
    GiftiLabelTable myTable;
    if (listfileName != "")//maybe this should be a function of GiftiLabelTable
    {
        AString temp;
        FileInformation textFileInfo(listfileName);
        if (!textFileInfo.exists())
        {
            throw OperationException("label list file doesn't exist");
        }
        fstream labelListFile(listfileName.toLocal8Bit().constData(), fstream::in);
        if (!labelListFile.good())
        {
            throw OperationException("error reading label list file");
        }
        string labelName;
        int32_t value, red, green, blue, alpha;
        translate[unlabeledValue] = 0;//placeholder, we don't know the correct translated value yet
        while (labelListFile.good())
        {
            getline(labelListFile, labelName);
            labelListFile >> value;
            labelListFile >> red;
            labelListFile >> green;
            labelListFile >> blue;
            if (!(labelListFile >> alpha))//yes, that is seriously the correct way to check if input was successfully extracted...so much fail
            {
                break;//stop at malformed lines
            }
            while (isspace(labelListFile.peek()))
            {
                labelListFile.ignore();//drop the newline, possible carriage return or other whitespace so that getline doesn't get nothing, and cause int extraction to fail
            }
            temp = AString(labelName.c_str()).trimmed();//drop errant CR or other whitespace from beginning and end of lines
            if (translate.find(value) != translate.end())
            {
                if (value == unlabeledValue)
                {
                    throw OperationException("the unlabeled value must not be specified in label list file");
                } else {
                    throw OperationException(AString("label key ") + AString::number(value) + " specified more than once");
                }
            }
            GiftiLabel myLabel(value, temp, red, green, blue, alpha);
            if (myTable.getLabelKeyFromName(temp) != GiftiLabel::getInvalidLabelKey())
            {
                AString nameBase = temp, newName;//resolve collision by generating a name with an additional number on it
                bool success = false;
                for (int extra = 1; extra < 100; ++extra)//but stop at 100, because really...
                {
                    newName = nameBase + "_" + AString::number(extra);
                    if (myTable.getLabelKeyFromName(newName) == GiftiLabel::getInvalidLabelKey())
                    {
                        success = true;
                        break;
                    }
                }
                if (success)
                {
                    CaretLogWarning("name collision in input name '" + nameBase + "', changing one to '" + newName + "'");
                } else {
                    throw OperationException("giving up on resolving name collision for input name '" + nameBase + "'");
                }
                myLabel.setName(newName);
            }
            int32_t newValue;
            if (value == 0)//because label 0 exists in the default constructed table
            {
                myTable.insertLabel(&myLabel);//but we do want to be able to overwrite the default 0 label
                newValue = 0;//if value 0 is specified twice, or once without specifying a different unlabeled value, the check versus the translate map will catch it
            } else {
                newValue = myTable.addLabel(&myLabel);//we don't want to overwrite relocated labels
            }
            translate[value] = newValue;
        }
    }
    int32_t unusedLabel = myTable.getUnassignedLabelKey();
    translate[unlabeledValue] = unusedLabel;
    const int numNodes = myMetric->getNumberOfNodes();
    vector<int32_t> colScratch(numNodes);
    if (columnNum == -1)
    {
        const int numCols = myMetric->getNumberOfColumns();
        myLabelOut->setNumberOfNodesAndColumns(numNodes, numCols);
        myLabelOut->setStructure(myMetric->getStructure());
        set<int32_t> usedValues;
        for (int col = 0; col < numCols; ++col)
        {
            translateLabels(myMetric->getValuePointerForColumn(col), colScratch.data(), numNodes, myTable, translate, usedValues, dropUnused, discardOthers, unusedLabel);
            myLabelOut->setLabelKeysForColumn(col, colScratch.data());
        }
        if (dropUnused)
        {
            myTable.deleteUnusedLabels(usedValues);
        }
        *(myLabelOut->getLabelTable()) = myTable;
    } else {
        myLabelOut->setNumberOfNodesAndColumns(numNodes, 1);
        myLabelOut->setStructure(myMetric->getStructure());
        set<int32_t> usedValues;
        translateLabels(myMetric->getValuePointerForColumn(columnNum), colScratch.data(), numNodes, myTable, translate, usedValues, dropUnused, discardOthers, unusedLabel);
        myLabelOut->setLabelKeysForColumn(0, colScratch.data());
        if (dropUnused)
        {
            myTable.deleteUnusedLabels(usedValues);
        }
        *(myLabelOut->getLabelTable()) = myTable;
    }
}

void OperationMetricLabelImport::translateLabels(const float* valuesIn, int32_t* labelsOut, const int& numNodes, GiftiLabelTable& myTable, map<int32_t, int32_t>& translate,
                                                 set<int32_t>& usedValues, const bool& dropUnused, const bool& discardOthers, const int32_t& unusedLabel)
{
    for (int node = 0; node < numNodes; ++node)
    {
        int32_t labelval = (int32_t)floor(valuesIn[node] + 0.5f);//just in case it somehow got poorly encoded, round to nearest
        if (dropUnused)
        {
            usedValues.insert(labelval);
        }
        map<int32_t, int32_t>::iterator myiter = translate.find(labelval);
        if (myiter == translate.end())
        {
            if (discardOthers)
            {
                labelsOut[node] = unusedLabel;
            } else {//use a random color, but fully opaque for the label
                GiftiLabel myLabel(labelval, AString("LABEL_") + AString::number(labelval), rand() & 255, rand() & 255, rand() & 255, 255);
                if (myTable.getLabelKeyFromName(myLabel.getName()) != GiftiLabel::getInvalidLabelKey())
                {
                    AString nameBase = myLabel.getName(), newName;//resolve collision by generating a name with an additional number on it
                    bool success = false;
                    for (int extra = 1; extra < 100; ++extra)//but stop at 100, because really...
                    {
                        newName = nameBase + "_" + AString::number(extra);
                        if (myTable.getLabelKeyFromName(newName) == GiftiLabel::getInvalidLabelKey())
                        {
                            success = true;
                            break;
                        }
                    }
                    if (success)
                    {
                        CaretLogWarning("name collision in auto-generated name '" + nameBase + "', changed to '" + newName + "'");
                    } else {
                        throw OperationException("giving up on resolving name collision for auto-generated name '" + nameBase + "'");
                    }
                    myLabel.setName(newName);
                }
                int32_t newValue = myTable.addLabel(&myLabel);//don't overwrite any values in the table
                translate[labelval] = newValue;
                labelsOut[node] = newValue;
            }
        } else {
            labelsOut[node] = myiter->second;
        }
    }
}