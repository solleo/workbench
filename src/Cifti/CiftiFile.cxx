/*LICENSE_START*/
/*
 *  Copyright (C) 2014  Washington University School of Medicine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*LICENSE_END*/

#include "CiftiFile.h"

#include "CaretAssert.h"
#include "CaretLogger.h"
#include "DataFileException.h"
#include "FileInformation.h"
#include "MultiDimArray.h"
#include "MultiDimIterator.h"
#include "NiftiIO.h"

using namespace std;
using namespace caret;

//private implementation classes
namespace caret
{
    class CiftiOnDiskImpl : public CiftiFile::WriteImplInterface
    {
        mutable NiftiIO m_nifti;//because file objects aren't stateless (current position), so reading "changes" them
        CiftiXML m_xml;//because we need to parse it to set up the dimensions anyway
    public:
        CiftiOnDiskImpl(const QString& filename);//read-only
        CiftiOnDiskImpl(const QString& filename, const CiftiXML& xml, const CiftiVersion& version);//make new empty file with read/write
        void getRow(float* dataOut, const std::vector<int64_t>& indexSelect, const bool& tolerateShortRead) const;
        void getColumn(float* dataOut, const int64_t& index) const;
        const CiftiXML& getCiftiXML() const { return m_xml; }
        QString getFilename() const { return m_nifti.getFilename(); }
        void setRow(const float* dataIn, const std::vector<int64_t>& indexSelect);
        void setColumn(const float* dataIn, const int64_t& index);
    };
    
    class CiftiMemoryImpl : public CiftiFile::WriteImplInterface
    {
        MultiDimArray<float> m_array;
    public:
        CiftiMemoryImpl(const CiftiXML& xml);
        void getRow(float* dataOut, const std::vector<int64_t>& indexSelect, const bool& tolerateShortRead) const;
        void getColumn(float* dataOut, const int64_t& index) const;
        bool isInMemory() const { return true; }
        void setRow(const float* dataIn, const std::vector<int64_t>& indexSelect);
        void setColumn(const float* dataIn, const int64_t& index);
    };
}

CiftiFile::ReadImplInterface::~ReadImplInterface()
{
}

CiftiFile::WriteImplInterface::~WriteImplInterface()
{
}

CiftiFile::CiftiFile(const QString& fileName)
{
    openFile(fileName);
}

void CiftiFile::openFile(const QString& fileName)
{
    m_writingImpl.grabNew(NULL);
    m_readingImpl.grabNew(NULL);//to make sure it closes everything first, even if the open throws
    m_dims.clear();
    CaretPointer<CiftiOnDiskImpl> newRead(new CiftiOnDiskImpl(FileInformation(fileName).getAbsoluteFilePath()));//this constructor opens existing file read-only
    m_readingImpl = newRead;//it should be noted that if the constructor throws (if the file isn't readable), new guarantees the memory allocated for the object will be freed
    m_xml = newRead->getCiftiXML();
    m_dims = m_xml.getDimensions();
}

void CiftiFile::setWritingFile(const QString& fileName)
{
    m_writingFile = FileInformation(fileName).getAbsoluteFilePath();//always resolve paths as soon as they enter CiftiFile, in case some clown changes directory before writing data
    m_writingImpl.grabNew(NULL);//prevent writing to previous writing implementation, let the next set...() set up for writing
}

void CiftiFile::writeFile(const QString& fileName, const CiftiVersion& writingVersion)
{
    if (m_readingImpl == NULL || m_dims.empty()) throw DataFileException("writeFile called on uninitialized CiftiFile");
    FileInformation myInfo(fileName);
    QString canonicalFilename = myInfo.getCanonicalFilePath();//NOTE: returns EMPTY STRING for nonexistant file
    const CiftiOnDiskImpl* testImpl = dynamic_cast<CiftiOnDiskImpl*>(m_readingImpl.getPointer());
    bool collision = false;
    if (testImpl != NULL && canonicalFilename != "" && FileInformation(testImpl->getFilename()).getCanonicalFilePath() == canonicalFilename)
    {
        collision = true;//empty string test is so that we don't say collision if both are nonexistant - could happen if file is removed/unlinked while reading on some filesystems
    }
    if (collision)
    {
        if (m_writingVersion == writingVersion) return;//don't need to copy to itself
        convertToInMemory();//otherwise, we need to preserve the contents first - if writing fails, we will end up with it converted to in-memory, but oh well
    }
    CaretPointer<WriteImplInterface> tempWrite(new CiftiOnDiskImpl(myInfo.getAbsoluteFilePath(), m_xml, writingVersion));
    vector<int64_t> iterateDims(m_dims.begin() + 1, m_dims.end());//above constructor creates new file in read/write mode
    vector<float> scratchRow(m_dims[0]);
    for (MultiDimIterator<int64_t> iter(iterateDims); !iter.atEnd(); ++iter)
    {
        m_readingImpl->getRow(scratchRow.data(), *iter, false);
        tempWrite->setRow(scratchRow.data(), *iter);
    }
    if (collision)//drop the in-memory representation afterwards
    {
        m_writingVersion = writingVersion;//also record the current version number
        m_writingImpl = tempWrite;
        m_readingImpl = tempWrite;
    }
}

void CiftiFile::writeFile(const QString& fileName)
{
    writeFile(fileName, m_writingVersion);//let the more complex case handle the simple one too, will always return early on collision
}

void CiftiFile::convertToInMemory()
{
    if (isInMemory()) return;
    if (m_readingImpl == NULL  || m_dims.empty())//not set up yet
    {
        m_writingFile = "";//make sure it doesn't do on-disk when set...() is called
        return;
    }
    CaretPointer<WriteImplInterface> tempWrite(new CiftiMemoryImpl(m_xml));//if we get an error while reading, free the memory immediately
    vector<int64_t> iterateDims(m_dims.begin() + 1, m_dims.end());
    vector<float> scratchRow(m_dims[0]);
    for (MultiDimIterator<int64_t> iter(iterateDims); !iter.atEnd(); ++iter)
    {
        m_readingImpl->getRow(scratchRow.data(), *iter, false);
        tempWrite->setRow(scratchRow.data(), *iter);
    }
    m_writingImpl = tempWrite;
    m_readingImpl = tempWrite;
}

bool CiftiFile::isInMemory() const
{
    if (m_readingImpl == NULL)
    {
        return (m_writingFile == "");//return what it would be if verifyImpl() was called
    } else {
        return m_readingImpl->isInMemory();
    }
}

void CiftiFile::getRow(float* dataOut, const vector<int64_t>& indexSelect, const bool& tolerateShortRead) const
{
    if (m_dims.empty()) throw DataFileException("getRow called on uninitialized CiftiFile");
    if (m_readingImpl == NULL) return;//NOT an error because we are pretending to have a matrix already, while we are waiting for setRow to actually start writing the file
    m_readingImpl->getRow(dataOut, indexSelect, tolerateShortRead);
}

void CiftiFile::getColumn(float* dataOut, const int64_t& index) const
{
    if (m_dims.empty()) throw DataFileException("getColumn called on uninitialized CiftiFile");
    if (m_dims.size() != 2) throw DataFileException("getColumn called on non-2D CiftiFile");
    if (m_readingImpl == NULL) return;//NOT an error because we are pretending to have a matrix already, while we are waiting for setRow to actually start writing the file
    m_readingImpl->getColumn(dataOut, index);
}

void CiftiFile::setCiftiXML(const CiftiXML& xml, const bool useOldMetadata, const CiftiVersion& writingVersion)
{
    if (xml.getNumberOfDimensions() == 0) throw DataFileException("setCiftiXML called with 0-dimensional CiftiXML");
    m_writingVersion = writingVersion;
    if (useOldMetadata)
    {
        const GiftiMetaData* oldmd = m_xml.getFileMetaData();
        if (oldmd != NULL)
        {
            GiftiMetaData newmd = *oldmd;//make a copy
            oldmd = NULL;//don't leave a potentially dangling pointer around
            m_xml = xml;//because this will result in a new pointer for the metadata
            GiftiMetaData* changemd = m_xml.getFileMetaData();
            if (changemd != NULL)
            {
                *changemd = newmd;
            }
        } else {
            m_xml = xml;
        }
    } else {
        m_xml = xml;
    }
    m_dims = m_xml.getDimensions();
    m_readingImpl.grabNew(NULL);//drop old implementation, as it is now invalid due to XML (and therefore matrix size) change
    m_writingImpl.grabNew(NULL);
}

void CiftiFile::setCiftiXML(const CiftiXMLOld& xml, const bool useOldMetadata, const CiftiVersion& writingVersion)
{
    QString xmlText;
    xml.writeXML(xmlText);
    CiftiXML tempXML;//so that we can use the same code path
    tempXML.readXML(xmlText);
    if (tempXML.getDimensionLength(CiftiXML::ALONG_ROW) < 1)
    {
        CiftiSeriesMap& tempMap = tempXML.getSeriesMap(CiftiXML::ALONG_ROW);
        tempMap.setLength(xml.getDimensionLength(CiftiXMLOld::ALONG_ROW));
    }
    if (tempXML.getDimensionLength(CiftiXML::ALONG_COLUMN) < 1)
    {
        CiftiSeriesMap& tempMap = tempXML.getSeriesMap(CiftiXML::ALONG_COLUMN);
        tempMap.setLength(xml.getDimensionLength(CiftiXMLOld::ALONG_COLUMN));
    }
    setCiftiXML(tempXML, useOldMetadata, writingVersion);
}

void CiftiFile::setRow(const float* dataIn, const vector<int64_t>& indexSelect)
{
    verifyWriteImpl();
    m_writingImpl->setRow(dataIn, indexSelect);
}

void CiftiFile::setColumn(const float* dataIn, const int64_t& index)
{
    verifyWriteImpl();
    if (m_dims.size() != 2) throw DataFileException("setColumn called on non-2D CiftiFile");
    m_writingImpl->setColumn(dataIn, index);
}

//compatibility with old interface
void CiftiFile::getRow(float* dataOut, const int64_t& index, const bool& tolerateShortRead) const
{
    if (m_dims.empty()) throw DataFileException("getRow called on uninitialized CiftiFile");
    if (m_dims.size() != 2) throw DataFileException("getRow with single index called on non-2D CiftiFile");
    if (m_readingImpl == NULL) return;//NOT an error because we are pretending to have a matrix already, while we are waiting for setRow to actually start writing the file
    vector<int64_t> tempvec(1, index);//could use a member if we need more speed
    m_readingImpl->getRow(dataOut, tempvec, tolerateShortRead);
}

void CiftiFile::getRow(float* dataOut, const int64_t& index) const
{
    getRow(dataOut, index, false);//once CiftiInterface is gone, we can collapse this into a default value
}

int64_t CiftiFile::getNumberOfRows() const
{
    if (m_dims.empty()) throw DataFileException("getNumberOfRows called on uninitialized CiftiFile");
    if (m_dims.size() != 2) throw DataFileException("getNumberOfRows called on non-2D CiftiFile");
    return m_dims[1];//length of a column
}

int64_t CiftiFile::getNumberOfColumns() const
{
    if (m_dims.empty()) throw DataFileException("getNumberOfRows called on uninitialized CiftiFile");
    if (m_dims.size() != 2) throw DataFileException("getNumberOfRows called on non-2D CiftiFile");
    return m_dims[0];//length of a row
}

void CiftiFile::setRow(const float* dataIn, const int64_t& index)
{
    verifyWriteImpl();
    if (m_dims.size() != 2) throw DataFileException("setRow with single index called on non-2D CiftiFile");
    vector<int64_t> tempvec(1, index);//could use a member if we need more speed
    m_writingImpl->setRow(dataIn, tempvec);
}
//*///end old compatibility functions

void CiftiFile::verifyWriteImpl()
{//this is where the magic happens - we want to emulate being a simple in-memory file, but actually be reading/writing on-disk when possible
    if (m_writingImpl != NULL) return;
    CaretAssert(!m_dims.empty());//if the xml hasn't been set, then we can't do anything meaningful
    if (m_dims.empty()) throw DataFileException("setRow or setColumn attempted on uninitialized CiftiFile");
    if (m_writingFile == "")
    {
        if (m_readingImpl != NULL)
        {
            convertToInMemory();
        } else {
            m_writingImpl.grabNew(new CiftiMemoryImpl(m_xml));
        }
    } else {
        if (m_readingImpl != NULL)
        {
            CiftiOnDiskImpl* testImpl = dynamic_cast<CiftiOnDiskImpl*>(m_readingImpl.getPointer());
            if (testImpl != NULL && testImpl->getFilename() == m_writingFile)//these have already been canonicalized, see setWritingFile(), openFile()
            {
                convertToInMemory();//save existing data in memory before we clobber file
            }
        }
        m_writingImpl.grabNew(new CiftiOnDiskImpl(m_writingFile, m_xml, m_writingVersion));//this constructor makes new file for writing
        if (m_readingImpl != NULL)
        {
            vector<int64_t> iterateDims(m_dims.begin() + 1, m_dims.end());
            vector<float> scratchRow(m_dims[0]);
            for (MultiDimIterator<int64_t> iter(iterateDims); !iter.atEnd(); ++iter)
            {
                m_readingImpl->getRow(scratchRow.data(), *iter, false);
                m_writingImpl->setRow(scratchRow.data(), *iter);
            }
        }
    }
    m_readingImpl = m_writingImpl;//read-only implementations are set up in specialized functions
}

CiftiMemoryImpl::CiftiMemoryImpl(const CiftiXML& xml)
{
    CaretAssert(xml.getNumberOfDimensions() != 0);
    m_array.resize(xml.getDimensions());
}

void CiftiMemoryImpl::getRow(float* dataOut, const vector<int64_t>& indexSelect, const bool&) const
{
    const float* ref = m_array.get(1, indexSelect);
    int64_t rowSize = m_array.getDimensions()[0];//we don't accept 0-D CiftiXML, so this will always work
    for (int64_t i = 0; i < rowSize; ++i)
    {
        dataOut[i] = ref[i];
    }
}

void CiftiMemoryImpl::getColumn(float* dataOut, const int64_t& index) const
{
    CaretAssert(m_array.getDimensions().size() == 2);//otherwise, CiftiFile shouldn't have called this
    const float* ref = m_array.get(2, vector<int64_t>());//empty vector is intentional, only 2 dimensions exist, so no more to select from
    int64_t rowSize = m_array.getDimensions()[0];
    int64_t colSize = m_array.getDimensions()[1];
    CaretAssert(index >= 0 && index < rowSize);//because we are doing the indexing math manually for speed
    for (int64_t i = 0; i < colSize; ++i)
    {
        dataOut[i] = ref[index + rowSize * i];
    }
}

void CiftiMemoryImpl::setRow(const float* dataIn, const vector< int64_t >& indexSelect)
{
    float* ref = m_array.get(1, indexSelect);
    int64_t rowSize = m_array.getDimensions()[0];//we don't accept 0-D CiftiXML, so this will always work
    for (int64_t i = 0; i < rowSize; ++i)
    {
        ref[i] = dataIn[i];
    }
}

void CiftiMemoryImpl::setColumn(const float* dataIn, const int64_t& index)
{
    CaretAssert(m_array.getDimensions().size() == 2);//otherwise, CiftiFile shouldn't have called this
    float* ref = m_array.get(2, vector<int64_t>());//empty vector is intentional, only 2 dimensions exist, so no more to select from
    int64_t rowSize = m_array.getDimensions()[0];
    int64_t colSize = m_array.getDimensions()[1];
    CaretAssert(index >= 0 && index < rowSize);//because we are doing the indexing math manually for speed
    for (int64_t i = 0; i < colSize; ++i)
    {
        ref[index + rowSize * i] = dataIn[i];
    }
}

CiftiOnDiskImpl::CiftiOnDiskImpl(const QString& filename)
{//opens existing file for reading
    m_nifti.openRead(filename);//read-only, so we don't need write permission to read a cifti file
    const NiftiHeader& myHeader = m_nifti.getHeader();
    int numExts = (int)myHeader.m_extensions.size(), whichExt = -1;
    for (int i = 0; i < numExts; ++i)
    {
        if (myHeader.m_extensions[i]->m_ecode == NIFTI_ECODE_CIFTI)
        {
            whichExt = i;
            break;
        }
    }
    if (whichExt == -1) throw DataFileException("no cifti extension found in file '" + filename + "'");
    m_xml.readXML(QByteArray(myHeader.m_extensions[whichExt]->m_bytes.data(), myHeader.m_extensions[whichExt]->m_bytes.size()));//CiftiXML should be under 2GB
    vector<int64_t> dimCheck = m_nifti.getDimensions();
    if (dimCheck.size() < 5) throw DataFileException("invalid dimensions in cifti file '" + filename + "'");
    for (int i = 0; i < 4; ++i)
    {
        if (dimCheck[i] != 1) throw DataFileException("non-singular dimension #" + QString::number(i + 1) + " in cifti file '" + filename + "'");
    }
    if (m_xml.getParsedVersion().hasReversedFirstDims())
    {
        while (dimCheck.size() < 6) dimCheck.push_back(1);//just in case
        int64_t temp = dimCheck[4];//note: nifti dim[5] is the 5th dimension, index 4 in this vector
        dimCheck[4] = dimCheck[5];
        dimCheck[5] = temp;
        m_nifti.overrideDimensions(dimCheck);
    }
    if (m_xml.getNumberOfDimensions() + 4 != (int)dimCheck.size()) throw DataFileException("XML does not match number of nifti dimensions in file " + filename + "'");
    for (int i = 4; i < (int)dimCheck.size(); ++i)
    {
        if (m_xml.getDimensionLength(i - 4) < 1)//CiftiXML will only let this happen with cifti-1
        {
            m_xml.getSeriesMap(i - 4).setLength(dimCheck[i]);//and only in a series map
        } else {
            if (m_xml.getDimensionLength(i - 4) != dimCheck[i])
            {
                throw DataFileException("xml and nifti header disagree on matrix dimensions");
            }
        }
    }
}

CiftiOnDiskImpl::CiftiOnDiskImpl(const QString& filename, const CiftiXML& xml, const CiftiVersion& version)
{//starts writing new file
    NiftiHeader outHeader;
    outHeader.setDataType(NIFTI_TYPE_FLOAT32);//actually redundant currently, default is float32
    char intentName[16];
    int32_t intentCode = xml.getIntentInfo(version, intentName);
    outHeader.setIntent(intentCode, intentName);
    QByteArray xmlBytes = xml.writeXMLToQByteArray(version);
    CaretPointer<NiftiExtension> outExtension(new NiftiExtension());
    outExtension->m_ecode = NIFTI_ECODE_CIFTI;
    int numBytes = xmlBytes.size();
    outExtension->m_bytes.resize(numBytes);
    for (int i = 0; i < numBytes; ++i)
    {
        outExtension->m_bytes[i] = xmlBytes[i];
    }
    outHeader.m_extensions.push_back(outExtension);
    vector<int64_t> matrixDims = xml.getDimensions();
    vector<int64_t> niftiDims(4, 1);//the reserved space and time dims
    niftiDims.insert(niftiDims.end(), matrixDims.begin(), matrixDims.end());
    if (version.hasReversedFirstDims())
    {
        vector<int64_t> headerDims = niftiDims;
        while (headerDims.size() < 6) headerDims.push_back(1);//just in case
        int64_t temp = headerDims[4];
        headerDims[4] = headerDims[5];
        headerDims[5] = temp;
        outHeader.setDimensions(headerDims);//give the header the reversed dimensions
        m_nifti.writeNew(filename, outHeader, 2, true);
        m_nifti.overrideDimensions(niftiDims);//and then tell the nifti reader to use the correct dimensions
    } else {
        outHeader.setDimensions(niftiDims);
        m_nifti.writeNew(filename, outHeader, 2, true);
    }
    m_xml = xml;
}

void CiftiOnDiskImpl::getRow(float* dataOut, const vector<int64_t>& indexSelect, const bool& tolerateShortRead) const
{
    m_nifti.readData(dataOut, 5, indexSelect, tolerateShortRead);//5 means 4 reserved (space and time) plus the first cifti dimension
}

void CiftiOnDiskImpl::getColumn(float* dataOut, const int64_t& index) const
{
    CaretAssert(m_xml.getNumberOfDimensions() == 2);//otherwise this shouldn't be called
    CaretAssert(index >= 0 && index < m_xml.getDimensionLength(CiftiXML::ALONG_ROW));
    CaretLogFine("getColumn called on CiftiOnDiskImpl, this will be slow");//generate logging messages at a low priority
    vector<int64_t> indexSelect(2);
    indexSelect[0] = index;
    int64_t colLength = m_xml.getDimensionLength(CiftiXML::ALONG_COLUMN);
    for (int64_t i = 0; i < colLength; ++i)//assume if they really want getColumn on disk, they don't want their pagecache obliterated, so read it 1 element at a time
    {
        indexSelect[1] = i;
        m_nifti.readData(dataOut + i, 4, indexSelect);//4 means just the 4 reserved dimensions, so 1 element of the matrix
    }
}

void CiftiOnDiskImpl::setRow(const float* dataIn, const vector<int64_t>& indexSelect)
{
    m_nifti.writeData(dataIn, 5, indexSelect);
}

void CiftiOnDiskImpl::setColumn(const float* dataIn, const int64_t& index)
{
    CaretAssert(m_xml.getNumberOfDimensions() == 2);//otherwise this shouldn't be called
    CaretAssert(index >= 0 && index < m_xml.getDimensionLength(CiftiXML::ALONG_ROW));
    CaretLogFine("getColumn called on CiftiOnDiskImpl, this will be slow");//generate logging messages at a low priority
    vector<int64_t> indexSelect(2);
    indexSelect[0] = index;
    int64_t colLength = m_xml.getDimensionLength(CiftiXML::ALONG_COLUMN);
    for (int64_t i = 0; i < colLength; ++i)//don't do RMW, so write it 1 element at a time
    {
        indexSelect[1] = i;
        m_nifti.writeData(dataIn + i, 4, indexSelect);//4 means just the 4 reserved dimensions, so 1 element of the matrix
    }
}
