#!/bin/bash

astyle --suffix=none --style=attach --indent=tab --indent-switches --indent-classes --unpad-paren --keep-one-line-statements --break-closing-brackets --pad-header --pad-oper --recursive ./*.c ./*.h
