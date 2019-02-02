#include "rt_image_parts.h"

RTImageParts::RTImageParts()
{
}

RTImageParts::~RTImageParts()
{
}

void RTImageParts::addPart( unsigned int startRow, unsigned int startCol, unsigned int endRow, unsigned int endCol )
{
    PART part;
    part.startRow = startRow;
    part.startCol = startCol;
    part.endRow   = endRow;
    part.endCol   = endCol;
    List.push_back( part );
}

PART RTImageParts::getPart()
{
    QMutexLocker ml( &Mutex );
    PART retVal = List.last();
    List.removeLast();
    return retVal;
}

bool RTImageParts::isEmpty()
{
    QMutexLocker ml( &Mutex );
    bool retVal = List.empty();
    return retVal;
}
