/*
    microsoft-oms-auditd-plugin

    Copyright (c) Microsoft Corporation

    All rights reserved. 

    MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef AUOMS_STDIN_READER_H
#define AUOMS_STDIN_READER_H

#include "IO.h"

#include <cstdint>
#include <cstddef>

class StdinReader: public IOBase {
public:
    StdinReader(): IOBase(0), _size(0), _start_idx(0), _cur_idx(0) {
        SetNonBlock(true);
    }

    ssize_t ReadLine(char* buf, size_t buf_len, long timeout, std::function<bool()> fn);

private:
    std::array<char,10240> _data;
    size_t _size;
    size_t _start_idx;
    size_t _cur_idx;

    bool have_line();
    ssize_t get_data(long timeout, std::function<bool()> fn);
};


#endif //AUOMS_STDIN_READER_H
