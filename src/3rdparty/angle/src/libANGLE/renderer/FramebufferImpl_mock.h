//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferImpl_mock.h:
//   Defines a mock of the FramebufferImpl class.
//

#ifndef LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/FramebufferImpl.h"

namespace rx
{

class MockFramebufferImpl : public rx::FramebufferImpl
{
  public:
    MockFramebufferImpl() : rx::FramebufferImpl(gl::FramebufferState()) {}
    virtual ~MockFramebufferImpl() { destructor(); }

    MOCK_METHOD3(discard, gl::Error(const gl::Context *, size_t, const GLenum *));
    MOCK_METHOD3(invalidate, gl::Error(const gl::Context *, size_t, const GLenum *));
    MOCK_METHOD4(invalidateSub,
                 gl::Error(const gl::Context *, size_t, const GLenum *, const gl::Rectangle &));

    MOCK_METHOD2(clear, gl::Error(const gl::Context *, GLbitfield));
    MOCK_METHOD4(clearBufferfv, gl::Error(const gl::Context *, GLenum, GLint, const GLfloat *));
    MOCK_METHOD4(clearBufferuiv, gl::Error(const gl::Context *, GLenum, GLint, const GLuint *));
    MOCK_METHOD4(clearBufferiv, gl::Error(const gl::Context *, GLenum, GLint, const GLint *));
    MOCK_METHOD5(clearBufferfi, gl::Error(const gl::Context *, GLenum, GLint, GLfloat, GLint));

    MOCK_CONST_METHOD1(getImplementationColorReadFormat, GLenum(const gl::Context *));
    MOCK_CONST_METHOD1(getImplementationColorReadType, GLenum(const gl::Context *));
    MOCK_METHOD5(readPixels,
                 gl::Error(const gl::Context *, const gl::Rectangle &, GLenum, GLenum, void *));

    MOCK_CONST_METHOD2(getSamplePosition, gl::Error(size_t, GLfloat *));

    MOCK_METHOD5(blit,
                 gl::Error(const gl::Context *,
                           const gl::Rectangle &,
                           const gl::Rectangle &,
                           GLbitfield,
                           GLenum));

    MOCK_CONST_METHOD1(checkStatus, bool(const gl::Context *));

    MOCK_METHOD2(syncState, void(const gl::Context *, const gl::Framebuffer::DirtyBits &));

    MOCK_METHOD0(destructor, void());
};

inline ::testing::NiceMock<MockFramebufferImpl> *MakeFramebufferMock()
{
    ::testing::NiceMock<MockFramebufferImpl> *framebufferImpl =
        new ::testing::NiceMock<MockFramebufferImpl>();
    // TODO(jmadill): add ON_CALLS for other returning methods
    ON_CALL(*framebufferImpl, checkStatus(testing::_)).WillByDefault(::testing::Return(true));

    // We must mock the destructor since NiceMock doesn't work for destructors.
    EXPECT_CALL(*framebufferImpl, destructor()).Times(1).RetiresOnSaturation();

    return framebufferImpl;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_
