#ifndef RT_IMAGE_PARTS_H
#define RT_IMAGE_PARTS_H

#include <mutex>
#include <vector>

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

    // Takes the next part; returns false when there are none left
    bool getPart( PART &part );

private:    // variables
    std::vector<PART> List;
    std::mutex        Mutex;
};

#endif // RT_IMAGE_PARTS_H
