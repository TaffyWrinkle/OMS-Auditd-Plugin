/*
    microsoft-oms-auditd-plugin

    Copyright (c) Microsoft Corporation

    All rights reserved.

    MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef AUOMS_JSONMESSAGESINKBASE_H
#define AUOMS_JSONMESSAGESINKBASE_H

#include "MessageSinkBase.h"

#include <memory>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

class JSONMessageSinkBase: virtual public MessageSinkVirtBase {
public:
    JSONMessageSinkBase(): _buffer(0, JSON_BUFFER_SIZE), _writer(_buffer) {}

    virtual void AddBoolField(const std::string& name, bool value);
    virtual void AddInt32Field(const std::string& name, int32_t value);
    virtual void AddInt64Field(const std::string& name, int64_t value);
    virtual void AddDoubleField(const std::string& name, double value);
    virtual void AddTimeField(const std::string& name, uint64_t sec, uint32_t msec);
    virtual void AddTimestampField(const std::string& name, uint64_t sec, uint32_t msec);
    virtual void AddStringField(const std::string& name, const std::string& value);
    virtual void AddStringField(const std::string& name, const char* value_data, size_t value_size);

protected:
    static constexpr size_t JSON_BUFFER_SIZE = 64*1024;

    void reset();

    rapidjson::StringBuffer _buffer;
    rapidjson::Writer<rapidjson::StringBuffer> _writer;

};

#endif //AUOMS_JSONMESSAGESINKBASE_H
