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

bool RTImageParts::getPart( PART &part )
{
    std::lock_guard<std::mutex> lock( Mutex );
    if ( List.empty() )
    {
        return false;
    }
    part = List.back();
    List.pop_back();
    return true;
}
