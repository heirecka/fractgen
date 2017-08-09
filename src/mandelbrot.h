/* $Id$
 * ==========================================================================
 * ====                   FRACTAL GRAPHICS GENERATOR                     ====
 * ==========================================================================
 *
 * Copyright (C) 2003-2017 by Thomas Dreibholz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: dreibh@iem.uni-due.de
 */

#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <fractalalgorithminterface.h>


/**
  *@author Thomas Dreibholz
  */
class Mandelbrot : public FractalAlgorithmInterface  {
   public:
   Mandelbrot(const char* identifier = "Mandelbrot",
              const char* name       = "Mandelbrot z[i+1]=z[i]^2-c");
   ~Mandelbrot();

   virtual std::complex<double> defaultC1() const;
   virtual std::complex<double> defaultC2() const;
   virtual unsigned int calculatePoint(const unsigned int x,
                                       const unsigned int y);

   private:
   static Mandelbrot* Registration;
};

#endif
