#!/bin/bash

####
# microsoft-oms-auditd-plugin
#
# Copyright (c) Microsoft Corporation
#
# All rights reserved. 
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the ""Software""), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
####

ISSUE_WARNING=0

echo "Checking if required dependencies are installed..." 1>&2
if [ ! -e /sbin/auditd ]; then
        echo "  /sbin/auditd isn't installed" 1>&2
        ISSUE_WARNING=1
fi
if [ ! -e /sbin/audispd ]; then
        echo "  /sbin/audispd isn't installed" 1>&2
        ISSUE_WARNING=1
fi
/sbin/ldconfig -p | grep libaudit.so >/dev/null 2>&1
if [ $? -ne 0 ]; then
        echo "  libaudit.so isn't installed" 1>&2
        ISSUE_WARNING=1
fi
/sbin/ldconfig -p | grep libauparse.so >/dev/null 2>&1
if [ $? -ne 0 ]; then
        echo "  libauparse.so isn't installed" 1>&2
        ISSUE_WARNING=1
fi

if [ $ISSUE_WARNING -ne 0 ]; then
    echo "  Because the necessary dependencies are not installed, the auoms auditd plugin cannot be installed." 1>&2
    echo "      For Debian & Ubuntu, install the 'auditd' package." 1>&2
    echo "      For CentOS, RHEL & SLES, install the 'audit' package." 1>&2
    exit 1
fi