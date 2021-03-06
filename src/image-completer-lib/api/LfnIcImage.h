//
// Copyright 2010, Darren Lafreniere
// <http://www.lafarren.com/image-completer/>
//
// This file is part of lafarren.com's Image Completer.
//
// Image Completer is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Image Completer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Image Completer, named License.txt. If not, see
// <http://www.gnu.org/licenses/>.
//

#ifndef LFN_IC_IMAGE_H
#define LFN_IC_IMAGE_H

#include <iostream>
#include <string>

// PIXEL_DIMENSION may be defined by the build environment. If not
// specified, assume RGB and default to 3 channels.
#ifndef PIXEL_DIMENSION
#define PIXEL_DIMENSION 3
#endif

namespace LfnIc
{
	///
	/// Pixel type info helpers. Use Image::PixelInfo directly rather than these.
	///
	template<typename ImagePixelChannelType, int ImagePixelNumChannels>
	struct ImagePixelInfoBase
	{
		static const bool IS_24_BIT_RGB = false;
	};

	///
	/// Specialization for RGB unsigned char pixels
	///
	template<>
	struct ImagePixelInfoBase<unsigned char, 3>
	{
		static const bool IS_24_BIT_RGB = true;
	};


	///
	/// Image interface. Used for both reading the input image and
	/// writing the output image.
	///
	class Image
	{
	public:
		struct Pixel;

		/// Non-const method to initialize the image to the specified width
		/// and height. Any existing rgb data is not necessarily reserved.
		/// Returns true if there were no errors.
		virtual bool Init(int width, int height) = 0;

		/// Returns true if the image is of valid dimensions with a valid
		/// rgb buffer.
		virtual bool IsValid() const = 0;

		/// Returns the image's file path, if any.
		virtual const std::string& GetFilePath() const = 0;

		/// Non-const and const access to the image's Rgb buffer. The
		/// result points to the upperleft-most pixel and the buffer is
		/// stored in row-major order, where the start of each row is
		/// separated by GetStride() bytes.
		virtual Pixel* GetData() = 0;
		virtual const Pixel* GetData() const = 0;

		/// Returns the image's width.
		virtual int GetWidth() const = 0;

		/// Returns the image's height.
		virtual int GetHeight() const = 0;

		/// Structure of a single image pixel.
		struct Pixel
		{
#ifdef USE_FLOAT_PIXELS
			typedef float ChannelType;
#else
			typedef unsigned char ChannelType;
#endif
			static const int NUM_CHANNELS = PIXEL_DIMENSION;

			ChannelType channel[NUM_CHANNELS];
		};

		struct PixelInfo : public ImagePixelInfoBase<Pixel::ChannelType, Pixel::NUM_CHANNELS>
		{
		};

	protected:
		// Instances cannot be destroyed through a base Image pointer
		// using delete.
		virtual ~Image() {}
	};
}

#endif
