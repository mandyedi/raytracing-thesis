#ifndef RT_IMAGE_PARTS_H
#define RT_IMAGE_PARTS_H

#include <QList>
#include <QMutex>

struct PART {
    unsigned int startRow;
    unsigned int startCol;
    unsigned int endRow;
    unsigned int endCol;
};

class RTImageParts
{
public:     // functions
    RTImageParts();
    ~RTImageParts();

    void addPart( unsigned int startRow, unsigned int startCol, unsigned int endRow, unsigned int endCol );
    PART getPart();
    bool isEmpty();

private:    // variables
    QList<PART> List;
    QMutex      Mutex;
};

#endif // RT_IMAGE_PARTS_H
