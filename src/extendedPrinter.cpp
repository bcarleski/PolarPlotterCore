/*
    This file is part of the PolarPlotterCore library.
    Copyright (c) 2024 Benjamin Carleski

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "extendedPrinter.h"

ExtendedPrinter::ExtendedPrinter(Print &printer)
    : printer(printer)
{
}

size_t ExtendedPrinter::write(uint8_t c)
{
    return this->printer.write(c);
}

size_t ExtendedPrinter::write(const uint8_t *str, size_t len)
{
    return this->printer.write(str, len);
}

void ExtendedPrinter::print(const char name[], const Point &value)
{
    String str = name;
    this->print(str, value);
}

void ExtendedPrinter::print(const String &name, const Point &value)
{
    this->printer.print(name);
    this->printer.print("=(");
    this->printer.print(value.getX(), 4);
    this->printer.print(", ");
    this->printer.print(value.getY(), 4);
    this->printer.print(", ");
    this->printer.print(value.getRadius(), 4);
    this->printer.print(", ");
    this->printer.print(value.getAzimuth(), 4);
    this->printer.print(")");
}

void ExtendedPrinter::println(const char name[], const Point &value)
{
    String str = name;
    this->println(str, value);
}

void ExtendedPrinter::println(const String &name, const Point &value)
{
    this->print(name, value);
    this->printer.println();
}