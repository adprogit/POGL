#include "matrix4.hh"

namespace mygl
{
    vector3 vector3::operator^(const vector3& v) const
    {
        return vector3(b_ * v.c_ - c_ * v.b_, c_ * v.a_ - a_ * v.c_,
                       a_ * v.b_ - b_ * v.a_);
    }

    float vector3::operator*(const vector3& v) const
    {
        return a_ * v.a_ + b_ * v.b_ + c_ * v.c_;
    }

    void vector3::normalize()
    {
        auto norm = std::sqrt(a_ * a_ + b_ * b_ + c_ * c_);
        a_ /= norm;
        b_ /= norm;
        c_ /= norm;
    }

    matrix4::matrix4()
    {
        content_ = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    }

    matrix4::matrix4(GLfloat a, GLfloat b, GLfloat c, GLfloat d, GLfloat e,
                     GLfloat f, GLfloat g, GLfloat h, GLfloat i, GLfloat j,
                     GLfloat k, GLfloat l, GLfloat m, GLfloat n, GLfloat o,
                     GLfloat p)
    {
        content_ = { a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p };
    }

    matrix4::matrix4(std::array<GLfloat, 16> arr)
    {
        content_ = arr;
    }

    // Stockage column-major : content_[col * 4 + row]
    GLfloat matrix4::operator()(size_t row, size_t col) const
    {
        return content_[col * 4 + row];
    }
    GLfloat& matrix4::operator()(size_t row, size_t col)
    {
        return content_[col * 4 + row];
    }

    matrix4 matrix4::identity()
    {
        matrix4 m;
        m(0, 0) = 1.0f;
        m(1, 1) = 1.0f;
        m(2, 2) = 1.0f;
        m(3, 3) = 1.0f;
        return m;
    }

    void matrix4::operator*=(const matrix4& rhs)
    {
        std::array<GLfloat, 16> res{};
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < 4; ++j)
            {
                GLfloat sum = 0.0f;
                for (size_t k = 0; k < 4; ++k)
                {
                    sum += (*this)(i, k) * rhs(k, j);
                }
                res[j * 4 + i] = sum;
            }
        }
        content_ = res;
    }

    matrix4 matrix4::operator*(const matrix4& rhs) const
    {
        matrix4 res;
        for (size_t i = 0; i < 4; ++i)
        {
            for (size_t j = 0; j < 4; ++j)
            {
                GLfloat sum = 0.0f;
                for (size_t k = 0; k < 4; ++k)
                {
                    sum += (*this)(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }
        return res;
    }

    matrix4 look_at(const GLfloat& eyeX, const GLfloat& eyeY,
                    const GLfloat& eyeZ, const GLfloat& centerX,
                    const GLfloat& centerY, const GLfloat& centerZ,
                    const GLfloat& upX, const GLfloat& upY, const GLfloat& upZ)
    {
        vector3 F(centerX - eyeX, centerY - eyeY, centerZ - eyeZ);
        vector3 UP(upX, upY, upZ);

        F.normalize();
        UP.normalize();

        vector3 eye(eyeX, eyeY, eyeZ);
        vector3 s = F ^ UP;
        s.normalize();
        vector3 u = s ^ F;
        u.normalize();

        matrix4 M;
        M(0, 0) = s.a_;
        M(0, 1) = s.b_;
        M(0, 2) = s.c_;
        M(0, 3) = -(s * eye);
        M(1, 0) = u.a_;
        M(1, 1) = u.b_;
        M(1, 2) = u.c_;
        M(1, 3) = -(u * eye);
        M(2, 0) = -F.a_;
        M(2, 1) = -F.b_;
        M(2, 2) = -F.c_;
        M(2, 3) = (F * eye);
        M(3, 0) = 0.0f;
        M(3, 1) = 0.0f;
        M(3, 2) = 0.0f;
        M(3, 3) = 1.0f;
        return M;
    }

    matrix4 frustum(const GLfloat& left, const GLfloat& right,
                    const GLfloat& bottom, const GLfloat& top,
                    const GLfloat& nearVal, const GLfloat& farVal)
    {
        GLfloat a = 2.0f * nearVal / (right - left);
        GLfloat c = (right + left) / (right - left);
        GLfloat f = 2.0f * nearVal / (top - bottom);
        GLfloat g = (top + bottom) / (top - bottom);
        GLfloat k = -(farVal + nearVal) / (farVal - nearVal);
        GLfloat l = -(2.0f * farVal * nearVal) / (farVal - nearVal);

        matrix4 M;
        M(0, 0) = a;
        M(0, 1) = 0.0f;
        M(0, 2) = c;
        M(0, 3) = 0.0f;
        M(1, 0) = 0.0f;
        M(1, 1) = f;
        M(1, 2) = g;
        M(1, 3) = 0.0f;
        M(2, 0) = 0.0f;
        M(2, 1) = 0.0f;
        M(2, 2) = k;
        M(2, 3) = l;
        M(3, 0) = 0.0f;
        M(3, 1) = 0.0f;
        M(3, 2) = -1.0f;
        M(3, 3) = 0.0f;
        return M;
    }
    matrix4 translate(GLfloat tx, GLfloat ty, GLfloat tz)
    {
        matrix4 M = matrix4::identity();
        M(0, 3) = tx;
        M(1, 3) = ty;
        M(2, 3) = tz;
        return M;
    }

    matrix4 scale(GLfloat sx, GLfloat sy, GLfloat sz)
    {
        matrix4 M;
        M(0, 0) = sx;
        M(1, 1) = sy;
        M(2, 2) = sz;
        M(3, 3) = 1.0f;
        return M;
    }
    matrix4 matrix4::inverse() const
    {
        const GLfloat* m = content_.data();
        GLfloat inv[16];

        inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14]
            - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11]
            - m[13] * m[7] * m[10];
        inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14]
            + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11]
            + m[12] * m[7] * m[10];
        inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13]
            - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11]
            - m[12] * m[7] * m[9];
        inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13]
            + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10]
            + m[12] * m[6] * m[9];

        inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14]
            + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11]
            + m[13] * m[3] * m[10];
        inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14]
            - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11]
            - m[12] * m[3] * m[10];
        inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13]
            + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11]
            + m[12] * m[3] * m[9];
        inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13]
            - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10]
            - m[12] * m[2] * m[9];

        inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15]
            + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14]
            + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7]
            + m[12] * m[3] * m[6];
        inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13]
            - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7]
            - m[12] * m[3] * m[5];
        inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13]
            + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6]
            + m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10]
            + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7]
            + m[9] * m[3] * m[6];
        inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11]
            + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9]
            + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7]
            + m[8] * m[3] * m[5];
        inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10]
            + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        GLfloat det =
            m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        if (det == 0.0f)
        {
            return matrix4::identity();
        }

        GLfloat inv_det = 1.0f / det;
        matrix4 result;
        for (int c = 0; c < 4; c++)
        {
            for (int r = 0; r < 4; r++)
            {
                result(r, c) = inv[c * 4 + r] * inv_det;
            }
        }
        return result;
    }

} // namespace mygl

std::ostream& operator<<(std::ostream& out, const mygl::matrix4& m)
{
    for (size_t i = 0; i < 4; ++i) // row
    {
        for (size_t j = 0; j < 4; ++j) // col
        {
            out << m(i, j) << " ";
        }
        out << std::endl;
    }
    return out;
}