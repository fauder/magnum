/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "Texture.h"

#ifndef MAGNUM_TARGET_WEBGL
#include <Corrade/Containers/StringView.h>
#endif

#include "Magnum/GL/Context.h"
#include "Magnum/GL/Extensions.h"
#include "Magnum/GL/Implementation/maxTextureSize.h"
#include "Magnum/GL/Implementation/State.h"
#include "Magnum/GL/Implementation/TextureState.h"

#ifndef MAGNUM_TARGET_GLES
#include "Magnum/Image.h"
#include "Magnum/GL/BufferImage.h"
#endif

#if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
#include "Magnum/GL/TextureArray.h"
#include "Magnum/GL/CubeMapTexture.h"
#include "Magnum/GL/CubeMapTextureArray.h"
#endif

namespace Magnum { namespace GL {

namespace Implementation {

template<UnsignedInt dimensions> VectorTypeFor<dimensions, Int> maxTextureSize() {
    return VectorTypeFor<dimensions, Int>{Implementation::maxTextureSideSize()};
}

#ifndef MAGNUM_TARGET_GLES
template MAGNUM_GL_EXPORT Math::Vector<1, Int> maxTextureSize<1>();
#endif
template MAGNUM_GL_EXPORT Vector2i maxTextureSize<2>();

#if !(defined(MAGNUM_TARGET_WEBGL) && defined(MAGNUM_TARGET_GLES2))
template<> MAGNUM_GL_EXPORT Vector3i maxTextureSize<3>() {
    #ifdef MAGNUM_TARGET_GLES2
    if(!Context::current().isExtensionSupported<Extensions::OES::texture_3D>())
        return {};
    #endif
    return {Vector2i(Implementation::maxTextureSideSize()), Implementation::max3DTextureDepth()};
}
#endif

}

#if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
template<UnsignedInt dimensions> Texture<dimensions> Texture<dimensions>::view(Texture<dimensions>& original, const TextureFormat internalFormat, const Int levelOffset, const Int levelCount) {
    /* glTextureView() doesn't work with glCreateTextures() as it needs an
       object without a name bound, so have to construct manually. The object
       is marked as Created as glTextureView() binds the name. */
    GLuint id;
    glGenTextures(1, &id);
    Texture<dimensions> out{id, ObjectFlag::Created|ObjectFlag::DeleteOnDestruction};
    out.viewInternal(original, internalFormat, levelOffset, levelCount, 0, 1);
    return out;
}

/* Other view() overloads at the end */
#endif

#ifndef MAGNUM_TARGET_GLES
template<UnsignedInt dimensions> Image<dimensions> Texture<dimensions>::image(const Int level, Image<dimensions>&& image) {
    this->image(level, image);
    return std::move(image);
}

template<UnsignedInt dimensions> BufferImage<dimensions> Texture<dimensions>::image(const Int level, BufferImage<dimensions>&& image, const BufferUsage usage) {
    this->image(level, image, usage);
    return std::move(image);
}

template<UnsignedInt dimensions> CompressedImage<dimensions> Texture<dimensions>::compressedImage(const Int level, CompressedImage<dimensions>&& image) {
    compressedImage(level, image);
    return std::move(image);
}

template<UnsignedInt dimensions> CompressedBufferImage<dimensions> Texture<dimensions>::compressedImage(const Int level, CompressedBufferImage<dimensions>&& image, const BufferUsage usage) {
    compressedImage(level, image, usage);
    return std::move(image);
}

template<UnsignedInt dimensions> Image<dimensions> Texture<dimensions>::subImage(const Int level, const RangeTypeFor<dimensions, Int>& range, Image<dimensions>&& image) {
    this->subImage(level, range, image);
    return std::move(image);
}

template<UnsignedInt dimensions> BufferImage<dimensions> Texture<dimensions>::subImage(const Int level, const RangeTypeFor<dimensions, Int>& range, BufferImage<dimensions>&& image, const BufferUsage usage) {
    this->subImage(level, range, image, usage);
    return std::move(image);
}

template<UnsignedInt dimensions> CompressedImage<dimensions> Texture<dimensions>::compressedSubImage(const Int level, const RangeTypeFor<dimensions, Int>& range, CompressedImage<dimensions>&& image) {
    compressedSubImage(level, range, image);
    return std::move(image);
}

template<UnsignedInt dimensions> CompressedBufferImage<dimensions> Texture<dimensions>::compressedSubImage(const Int level, const RangeTypeFor<dimensions, Int>& range, CompressedBufferImage<dimensions>&& image, const BufferUsage usage) {
    compressedSubImage(level, range, image, usage);
    return std::move(image);
}
#endif

#ifndef MAGNUM_TARGET_WEBGL
template<UnsignedInt dimensions> Texture<dimensions>& Texture<dimensions>::setLabel(Containers::StringView label) {
    AbstractTexture::setLabel(label);
    return *this;
}
#endif

#ifndef MAGNUM_TARGET_GLES
template class MAGNUM_GL_EXPORT Texture<1>;
#endif
#if !defined(MAGNUM_TARGET_GLES) || !defined(MAGNUM_TARGET_WEBGL)
template class MAGNUM_GL_EXPORT Texture<2>;
template class MAGNUM_GL_EXPORT Texture<3>;
#endif

#if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
/* Because these refer to concrete types different than the class itself, they
   have to be after the explicit type instantiations. Additionally, on Windows
   (MSVC, clang-cl and MinGw) these need an explicit export otherwise the
   symbol doesn't get exported.

   Other view() overloads at the top. */
#ifndef MAGNUM_TARGET_GLES
template<> template<> MAGNUM_GL_EXPORT Texture1D Texture1D::view(Texture1DArray& original, const TextureFormat internalFormat, const Int levelOffset, const Int levelCount, const Int layer) {
    /* glTextureView() doesn't work with glCreateTextures() as it needs an
       object without a name bound, so have to construct manually. The object
       is marked as Created as glTextureView() binds the name. */
    GLuint id;
    glGenTextures(1, &id);
    Texture1D out{id, ObjectFlag::Created|ObjectFlag::DeleteOnDestruction};
    out.viewInternal(original, internalFormat, levelOffset, levelCount, layer, 1);
    return out;
}
#endif

template<> template<> MAGNUM_GL_EXPORT Texture2D Texture2D::view(Texture2DArray& original, const TextureFormat internalFormat, const Int levelOffset, const Int levelCount, const Int layer) {
    /* glTextureView() doesn't work with glCreateTextures() as it needs an
       object without a name bound, so have to construct manually. The object
       is marked as Created as glTextureView() binds the name. */
    GLuint id;
    glGenTextures(1, &id);
    Texture2D out{id, ObjectFlag::Created|ObjectFlag::DeleteOnDestruction};
    out.viewInternal(original, internalFormat, levelOffset, levelCount, layer, 1);
    return out;
}

template<> template<> MAGNUM_GL_EXPORT Texture2D Texture2D::view(CubeMapTexture& original, const TextureFormat internalFormat, const Int levelOffset, const Int levelCount, const Int layer) {
    /* glTextureView() doesn't work with glCreateTextures() as it needs an
       object without a name bound, so have to construct manually. The object
       is marked as Created as glTextureView() binds the name. */
    GLuint id;
    glGenTextures(1, &id);
    Texture2D out{id, ObjectFlag::Created|ObjectFlag::DeleteOnDestruction};
    out.viewInternal(original, internalFormat, levelOffset, levelCount, layer, 1);
    return out;
}

template<> template<> MAGNUM_GL_EXPORT Texture2D Texture2D::view(CubeMapTextureArray& original, const TextureFormat internalFormat, const Int levelOffset, const Int levelCount, const Int layer) {
    /* glTextureView() doesn't work with glCreateTextures() as it needs an
       object without a name bound, so have to construct manually. The object
       is marked as Created as glTextureView() binds the name. */
    GLuint id;
    glGenTextures(1, &id);
    Texture2D out{id, ObjectFlag::Created|ObjectFlag::DeleteOnDestruction};
    out.viewInternal(original, internalFormat, levelOffset, levelCount, layer, 1);
    return out;
}
#endif

}}
