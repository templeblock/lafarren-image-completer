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

#ifndef OUTPUT_BLENDER_SOFT_MASK_H
#define OUTPUT_BLENDER_SOFT_MASK_H

#include "CompositorRoot.h"

namespace PriorityBp
{
	class ImageFloat;

	class OutputBlenderSoftMask : public CompositorRoot::OutputBlender
	{
	public:
		virtual void Blend(const Compositor::Input& input, const ImageFloat& patchesBlended, ImageFloat& outputImageFloat) const;
	};
}

#endif // OUTPUT_BLENDER_SOFT_MASK_H
