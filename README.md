# Easer
Easer (easy serialization) is a c++ library that handles serialization and streams objects.

## Usage
'''cpp
#include "easer/streamer.h"
#include <fstream>

struct MyStruct {
    BEGIN();
    FIELD(int, b);
    FIELD(bool, a);
    FIELD(char, c);
    END();
};

int main() 
{
    e    
}
