/**
 * Copyright (c) 2024 Yuri Sarudiansky
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "quantize.h"

#ifndef QUANTIZE_DISABLED

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

const double Quantize::ROTATION_BOUNDS = 0.707107;

const uint32_t Quantize::Quantize::MASK_A_9BIT = 511;              // 511 == 111111111   (9 bits per component)
const uint32_t Quantize::MASK_B_9BIT = 511 << 9;
const uint32_t Quantize::MASK_C_9BIT = 511 << 18;
const uint32_t Quantize::MASK_INDEX_9BIT = 3 << 27;      // 3 = 11
const uint32_t Quantize::MASK_SIGNAL_9BIT = 1 << 30;

const uint32_t Quantize::MASK_A_10BIT = 1023;            // 1023 = 1111111111    (10 bit per component)
const uint32_t Quantize::MASK_B_10BIT = 1023 << 10;
const uint32_t Quantize::MASK_C_10BIT = 1023 << 20;
const uint32_t Quantize::MASK_INDEX_10BIT = 3 << 30;

const uint32_t Quantize::MASK_A_15BIT = 32767;
const uint32_t Quantize::MASK_B_15BIT = 32767 << 15;
const uint32_t Quantize::MASK_C_15BIT = 32767;
const uint32_t Quantize::MASK_INDEX_15BIT = 3 << 30;
const uint32_t Quantize::MASK_SIGNAL_15BIT = 1 << 15;



void Quantize::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_static_method("Quantize", D_METHOD("quantize_unit_float", "value", "numbits"), &Quantize::quantize_unit_float);
   ClassDB::bind_static_method("Quantize", D_METHOD("restore_unit_float", "quantized", "numbits"), &Quantize::restore_unit_float);
   ClassDB::bind_static_method("Quantize", D_METHOD("quantize_float", "value", "minval", "maxval", "numbits"), &Quantize::quantize_float);
   ClassDB::bind_static_method("Quantize", D_METHOD("restore_float", "quantized", "minval", "maxval", "numbits"), &Quantize::restore_float);

   ClassDB::bind_static_method("Quantize", D_METHOD("compress_rotation_quat", "unit_quat", "numbits"), &Quantize::_compress_rotation_quaternion);
   ClassDB::bind_static_method("Quantize", D_METHOD("restore_rotation_quat", "quant", "numbits"), &Quantize::_restore_rotation_quaternion);

   ClassDB::bind_static_method("Quantize", D_METHOD("compress_rquat_9bits", "q"), &Quantize::compress_rquat_9bits);
   ClassDB::bind_static_method("Quantize", D_METHOD("restore_rquat_9bits", "compressed"), &Quantize::restore_rquat_9bits);

   ClassDB::bind_static_method("Quantize", D_METHOD("compress_rquat_10bits", "q"), &Quantize::compress_rquat_10bits);
   ClassDB::bind_static_method("Quantize", D_METHOD("restore_rquat_10bits", "compressed"), &Quantize::restore_rquat_10bits);

   ClassDB::bind_static_method("Quantize", D_METHOD("compress_rquat_15bits", "q"), &Quantize::compress_rquat_15bits);
   ClassDB::bind_static_method("Quantize", D_METHOD("restore_rquat_15bits", "pack0", "pack1"), &Quantize::_restore_rquat_15bits);
}


uint32_t Quantize::quantize_unit_float(double value, int num_bits)
{
   ERR_FAIL_COND_V_MSG(num_bits < 2 || num_bits > 32, -1, godot::vformat("Number of bits must be between 2 and 32, but %s has been given", num_bits));

   uint32_t intervals = 1 << num_bits;
   double scaled = value * (intervals - 1.0f);
   uint32_t rounded = (uint32_t)(scaled + 0.5f);

   if (rounded > intervals - 1)
   {
      rounded -= 1;
   }

   return rounded;
}

double Quantize::restore_unit_float(uint32_t quantized, int num_bits)
{
   uint32_t intervals = 1 << num_bits;
   double interval_size = 1.0f / (double)(intervals - 1.0f);
   double approx_float = (double)quantized * interval_size;
   return approx_float;
}


uint32_t Quantize::quantize_float(double value, double minval, double maxval, int num_bits)
{
   // First convert the given value into a unit-float. Yes, this may even further cause error
   double unit = (value - minval) / (maxval - minval);
   uint32_t quantized = quantize_unit_float(unit, num_bits);
   return quantized;
}

double Quantize::restore_float(uint32_t quantized, double minval, double maxval, int num_bits)
{
   double unit = restore_unit_float(quantized, num_bits);
   // Convert back into the [minval..maxval] range
   double approx_float = minval + (unit * (maxval - minval));
   return approx_float;
}


// NOTE: this assumes the incoming quaternion is a rotation one, which means it's a unit quaternion (length = 1).
Quantize::uquat_data Quantize::compress_rotation_quaternion(const godot::Quaternion& q, int num_bits)
{
   // Since it's not possible to iterate through the quaternion components through a loop, creating
   // this temporary array.
   double comps[4] = { q.x, q.y, q.z, q.w };
   uint32_t quant[3];           // Will hold the three quantized components
   int mindex = 0;              // Index of the largest component
   double mval = -1.0f;          // Value of the largest component
   double sig = 1.0f;            // Signal of the dropped component. Assume it's positive.

   // Locate the largest component, storing its absolute value as well as the index
   for (int i = 0; i < 4; i++)
   {
      double abval = godot::Math::abs<double>(comps[i]);

      if (abval > mval)
      {
         mval = abval;
         mindex = i;
      }
   }

   if (comps[mindex] < 0.0f)
   {
      sig = -1.0f;
   }

   // Quantize the smallest components
   for (int i = 0, c = 0; i < 4; i++)
   {
      if (i != mindex)
      {
         double fl = comps[i] * sig;
         quant[c++] = quantize_float(fl, -ROTATION_BOUNDS, ROTATION_BOUNDS, num_bits);
      }
   }

   return uquat_data(quant[0], quant[1], quant[2], mindex, sig == 1.0f ? 1 : 0);
}



godot::Dictionary Quantize::_compress_rotation_quaternion(const godot::Quaternion& q, int num_bits)
{
   godot::Dictionary ret;

   uquat_data quant = compress_rotation_quaternion(q, num_bits);

   ret["a"] = quant.a;
   ret["b"] = quant.b;
   ret["c"] = quant.c;
   ret["index"] = quant.index;
   ret["sig"] = quant.signal;

   return ret;
}


godot::Quaternion Quantize::restore_rotation_quaternion(const Quantize::uquat_data& quant, int num_bits)
{
   // Take signal from 0=negative|1=positive (easier bit packing) to -1,+1 for easier multiplication
   double sig = quant.signal == 1 ? 1.0f : -1.0f;

   // Restore the three smallest components (a, b and c)
   double ra = restore_float(quant.a, -ROTATION_BOUNDS, ROTATION_BOUNDS, num_bits) * sig;
   double rb = restore_float(quant.b, -ROTATION_BOUNDS, ROTATION_BOUNDS, num_bits) * sig;
   double rc = restore_float(quant.c, -ROTATION_BOUNDS, ROTATION_BOUNDS, num_bits) * sig;
   // Restore the dropped component
   double dropped = sqrtf(1.0f - ra*ra - rb*rb - rc*rc) * sig;

   godot::Quaternion ret = godot::Quaternion();

   switch (quant.index)
   {
      case 0:
         // X was dropped
         ret = godot::Quaternion(dropped, ra, rb, rc);
         break;
      
      case 1:
         // Y was dropped
         ret = godot::Quaternion(ra, dropped, rb, rc);
         break;
      
      case 2:
         // Z was dropped
         ret = godot::Quaternion(ra, rb, dropped, rc);
         break;
      
      case 3:
         // W was dropped
         ret = godot::Quaternion(ra, rb, rc, dropped);
         break;
   }


   return ret;
}


godot::Quaternion Quantize::_restore_rotation_quaternion(const godot::Dictionary& quant, int num_bits)
{
   // Variants don't have a conversion to uint8_t. So, casting the return value into 32 bit
   // just to tell the compiler which conversion to use.
   uquat_data q;
   q.a = quant.get("a", 0);
   q.b = quant.get("b", 0);
   q.c = quant.get("c", 0);
   q.index = quant.get("index", 0);
   q.signal = quant.get("signal", 1);

   return restore_rotation_quaternion(q, num_bits);
}



uint32_t Quantize::compress_rquat_9bits(const godot::Quaternion& q)
{
   // First compress normally
   uquat_data comp = compress_rotation_quaternion(q, 9);
   // Then pack the data
   return (((comp.signal << 30) & MASK_SIGNAL_9BIT) |
           ((comp.index << 27) & MASK_INDEX_9BIT) |
           ((comp.c << 18) & MASK_C_9BIT) |
           ((comp.b << 9) & MASK_B_9BIT) |
           (comp.a & MASK_A_9BIT));
}

godot::Quaternion Quantize::restore_rquat_9bits(uint32_t compressed)
{
   // To restore, must use dictionary, so create it
   uquat_data unpacked(
      compressed & MASK_A_9BIT,
      (compressed & MASK_B_9BIT) >> 9,
      (compressed & MASK_C_9BIT) >> 18,
      (compressed & MASK_INDEX_9BIT) >> 27,
      (compressed & MASK_SIGNAL_9BIT) >> 30);

   return restore_rotation_quaternion(unpacked, 9);
}


uint32_t Quantize::compress_rquat_10bits(const godot::Quaternion& q)
{
   uquat_data comp = compress_rotation_quaternion(q, 10);
   return (((comp.index << 30) & MASK_INDEX_10BIT) |
           ((comp.c << 20) & MASK_C_10BIT) |
           ((comp.b << 10) & MASK_B_10BIT) |
           (comp.a & MASK_A_10BIT));
}

godot::Quaternion Quantize::restore_rquat_10bits(uint32_t compressed)
{
   uquat_data unpacked(
      compressed & MASK_A_10BIT,
      (compressed & MASK_B_10BIT) >> 10,
      (compressed & MASK_C_10BIT) >> 20,
      (compressed & MASK_INDEX_10BIT) >> 30);

   return restore_rotation_quaternion(unpacked, 10);
}


godot::PackedInt32Array Quantize::compress_rquat_15bits(const godot::Quaternion& q)
{
   // Obtain the compressed data
   Quantize::uquat_data c = compress_rotation_quaternion(q, 15);

   godot::PackedInt32Array ret;
   ret.append(((c.index << 30) & MASK_INDEX_15BIT) |
              ((c.b << 15) & MASK_B_15BIT) |
              (c.a & MASK_A_15BIT));
   
   ret.append(((c.signal << 15) & MASK_SIGNAL_15BIT) | (c.c & MASK_C_15BIT));

   return ret;
}


godot::Quaternion Quantize::restore_rquat_15bits(uint32_t pack0, uint16_t pack1)
{
   uquat_data unpacked(
      pack0 & MASK_A_15BIT,
      (pack0 & MASK_B_15BIT) >> 15,
      pack1 & MASK_C_15BIT,
      (pack0 & MASK_INDEX_15BIT) >> 30,
      (pack1 & MASK_SIGNAL_15BIT) >> 15);

   return restore_rotation_quaternion(unpacked, 15);
}



#endif
