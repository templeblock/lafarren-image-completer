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

#include "Pch.h"
#include "Image.h"

#include "tech/ImageUtils.h"

#include "tech/DbgMem.h"

//
// Internal Image extension. Exists to give ImageScalable destruction permission.
//
namespace LfnIc
{
	class ImageConstInternal : public ImageConst
	{
	public:
		virtual ~ImageConstInternal() {}
	};
}

//
// Extends ImageConstInternal by delegating all calls to a Image instance.
//
namespace LfnIc
{
	class ImageConstDelegateToImage : public ImageConstInternal
	{
	public:
		ImageConstDelegateToImage(const Image& image);

		virtual const Rgb* GetRgb() const;
		virtual int GetWidth() const;
		virtual int GetHeight() const;

	private:
		// Hide copy constructor.
		ImageConstDelegateToImage(const ImageConstDelegateToImage&) : m_image(*reinterpret_cast<const Image*>(0xCDCDCDCD)) {}

		const Image& m_image;
	};
}

LfnIc::ImageConstDelegateToImage::ImageConstDelegateToImage(const Image& image) :
m_image(image)
{
}

const LfnIc::Image::Rgb* LfnIc::ImageConstDelegateToImage::GetRgb() const
{
	return m_image.GetRgb();
}

int LfnIc::ImageConstDelegateToImage::GetWidth() const
{
	return m_image.GetWidth();
}

int LfnIc::ImageConstDelegateToImage::GetHeight() const
{
	return m_image.GetHeight();
}

//
// Extends ImageConstInternal. Initializes its data from halving the resolution of
// an input Image instance
//
namespace LfnIc
{
	class ImageScaledDown : public ImageConstInternal
	{
	public:
		ImageScaledDown(const ImageConst& imageToScaleDown);
		virtual ~ImageScaledDown();

		virtual const Rgb* GetRgb() const;
		virtual int GetWidth() const;
		virtual int GetHeight() const;

	private:
		// Hide copy constructor.
		ImageScaledDown(const ImageScaledDown&) {}

		int m_width;
		int m_height;
		Rgb* m_rgb;
	};
}

LfnIc::ImageScaledDown::ImageScaledDown(const ImageConst& imageToScaleDown)
{
	// Alias to reduce wordiness.
	const int otherWidth = imageToScaleDown.GetWidth();
	const int otherHeight = imageToScaleDown.GetHeight();
	const Rgb* otherRgb = imageToScaleDown.GetRgb();

	// Downsample otherRgb into m_rgb by averaging 2x2 pixel blocks into 1 pixel.
	// The low resolution image is half that of the high resolution image.
	m_width = otherWidth / 2;
	m_height = otherHeight / 2;
	wxASSERT(m_width > 0 && m_height > 0);

	m_rgb = new Rgb[m_width * m_height];

	const int stride = m_width * sizeof(Rgb);
	const int otherStride = otherWidth * sizeof(Rgb);
	for (int y = 0, otherY = 0; y < m_height; ++y, otherY += 2)
	{
		wxASSERT(otherY < otherHeight);
		Rgb* rgbCurrent = LfnTech::GetRowMajorPointer(m_rgb, stride, 0, y);

		const Rgb* otherRgbCurrentUpper = LfnTech::GetRowMajorPointer(otherRgb, otherStride, 0, otherY);

		// If the bottom edge of the imageToScaleDown image has no lower row,
		// point at the upper one; it'll average to the same value
		// and avoids an extra conditional in the loop.
		const Rgb* otherRgbCurrentLower = ((otherY + 1) < otherHeight)
			? LfnTech::GetRowMajorPointer(otherRgb, otherStride, 0, otherY + 1)
			: otherRgbCurrentUpper;

		for (int x = 0, otherX = 0; x < m_width; ++x, ++rgbCurrent, otherX += 2, otherRgbCurrentUpper += 2, otherRgbCurrentLower += 2)
		{
			wxASSERT(otherX < otherWidth);

			float r = otherRgbCurrentUpper[0].r + otherRgbCurrentLower[0].r;
			float g = otherRgbCurrentUpper[0].g + otherRgbCurrentLower[0].g;
			float b = otherRgbCurrentUpper[0].b + otherRgbCurrentLower[0].b;

			// numPixelsToAverage is the number of high resolution pixels we're
			// collapsing/averaging into a single low resolution pixel. At most,
			// this will be four. So far the left column's upper and lower pixels
			// have been added. If we're not at the right edge of the high res
			// image (checked just below), then the right upper and lower pixels
			// will be added as well.
			float numPixelsToAverage = 2.0f;
			if ((otherX + 1) < otherWidth)
			{
				r += otherRgbCurrentUpper[1].r + otherRgbCurrentLower[1].r;
				g += otherRgbCurrentUpper[1].g + otherRgbCurrentLower[1].g;
				b += otherRgbCurrentUpper[1].b + otherRgbCurrentLower[1].b;
				numPixelsToAverage = 4.0f;
			}

			r /= numPixelsToAverage;
			g /= numPixelsToAverage;
			b /= numPixelsToAverage;

			wxASSERT(r >= 0.0f && r <= 255.0f);
			wxASSERT(g >= 0.0f && g <= 255.0f);
			wxASSERT(b >= 0.0f && b <= 255.0f);

			rgbCurrent->r = r;
			rgbCurrent->g = g;
			rgbCurrent->b = b;
		}
	}
}

LfnIc::ImageScaledDown::~ImageScaledDown()
{
	delete [] m_rgb;
}

const LfnIc::Image::Rgb* LfnIc::ImageScaledDown::GetRgb() const
{
	return m_rgb;
}

int LfnIc::ImageScaledDown::GetWidth() const
{
	return m_width;
}

int LfnIc::ImageScaledDown::GetHeight() const
{
	return m_height;
}

//
// ImageConst implementation
//
bool LfnIc::ImageConst::Init(int width, int height)
{
	// Not supported, and not expected to be called.
	wxASSERT(false);
	return false;
}

bool LfnIc::ImageConst::IsValid() const
{
	return true;
}

const std::string& LfnIc::ImageConst::GetFilePath() const
{
	static const std::string empty("");
	return empty;
}

LfnIc::Image::Rgb* LfnIc::ImageConst::GetRgb()
{
	// Not supported, and not expected to be called.
	wxASSERT(false);
	return NULL;
}

//
// ImageScalable implementation
//
LfnIc::ImageScalable::ImageScalable(const Image& image) :
m_depth(0)
{
	// Delegate to original resolution Image at depth 0.
	m_resolutions.push_back(new ImageConstDelegateToImage(image));
}

LfnIc::ImageScalable::~ImageScalable()
{
	for (int i = 0, n = m_resolutions.size(); i < n; ++i)
	{
		delete m_resolutions[i];
	}
}

const LfnIc::Image::Rgb* LfnIc::ImageScalable::GetRgb() const
{
	return GetCurrentResolution().GetRgb();
}

int LfnIc::ImageScalable::GetWidth() const
{
	return GetCurrentResolution().GetWidth();
}

int LfnIc::ImageScalable::GetHeight() const
{
	return GetCurrentResolution().GetHeight();
}

void LfnIc::ImageScalable::ScaleUp()
{
	wxASSERT(m_depth > 0);

	// We don't expect to scale back down to this resolution, so free up some
	// memory. This is checked by the assert at the bottom of
	// ImageScalable::ScaleDown().
	delete m_resolutions[m_depth];
	m_resolutions[m_depth] = NULL;

	--m_depth;
}

void LfnIc::ImageScalable::ScaleDown()
{
	wxASSERT(m_depth >= 0);

	// If there's no mask for the next lower depth, create one from the current resolution.
	if (static_cast<unsigned int>(m_depth) == m_resolutions.size() - 1)
	{
		const ImageConst& imageToScaleDown = GetCurrentResolution();
		m_resolutions.push_back(new ImageScaledDown(imageToScaleDown));
	}

	++m_depth;
	wxASSERT(m_depth < int(m_resolutions.size()));

	// ImageScalable::ScaleUp() doesn't expect us to scale back down, so it
	// deletes the lower resolution without reducing the array size. Verify
	// that we have a valid resolution here.
	wxASSERT(m_resolutions[m_depth]);
}

int LfnIc::ImageScalable::GetScaleDepth() const
{
	return m_depth;
}
