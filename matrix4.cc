#include "matrix4.hh"

namespace mygl
{
    vector3 vector3::operator^(const vector3& v) const
    {
        return vector3(b_ * v.c_ - c_ * v.b_, c_ * v.a_ - a_ * v.c_,
                       a_ * v.b_ - b_ * v.a_);
    }
    void vector3::normalize()
    {
        a_ /= std::sqrt(a_ * a_ + b_ * b_ + c_ * c_);
        b_ /= std::sqrt(a_ * a_ + b_ * b_ + c_ * c_);
        c_ /= std::sqrt(a_ * a_ + b_ * b_ + c_ * c_);
    }

    matrix4::matrix4(std::vector<std::vector<GLfloat>> content)
    {
        content_.insert(content_.begin(), content.begin(), content.end());
    }
    matrix4 matrix4::identity()
    {
        std::vector<std::vector<GLfloat>> v = { { 1.0, 0.0, 0.0, 0.0 },
                                                { 0.0, 1.0, 0.0, 0.0 },
                                                { 0.0, 0.0, 1.0, 0.0 },
                                                { 0.0, 0.0, 0.0, 1.0 } };

        matrix4 mt(v);
        return mt;
    }
    void matrix4::operator*=(const matrix4& rhs)
    {
        std::vector<std::vector<GLfloat>> res(4, std::vector<GLfloat>(4, 0.0f));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                res[i][j] = 0;
                for (int k = 0; k < 4; k++)
                {
                    res[i][j] += content_[i][k] * rhs.content_[k][j];
                }
            }
        }

        content_ = std::move(res);
    }
    matrix4 look_at(const GLfloat& eyeX, const GLfloat& eyeY,
                    const GLfloat& eyeZ, const GLfloat& centerX,
                    const GLfloat& centerY, const GLfloat& centerZ,
                    const GLfloat& upX, const GLfloat& upY, const GLfloat& upZ)
    {
        vector3 F(centerX - eyeX, centerY - eyeY, centerZ - eyeZ);
        vector3 UP(upX, upY, upZ);

        UP.normalize();
        F.normalize();

        vector3 s = F ^ UP;
        s.normalize();
        vector3 u = s ^ F;

        std::vector<std::vector<GLfloat>> m = { { s.a_, s.b_, s.c_, 0.0f },
                                                { u.a_, u.b_, u.c_, 0.0f },
                                                { -F.a_, F.b_, F.c_, 0.0f },
                                                { 0.0f, 0.0f, 0.0f, 1.0f } };
        matrix4 M(m);
        return M;
    }
    matrix4 frustum(const GLfloat& left, const GLfloat& right,
                    const GLfloat& bottom, const GLfloat& top,
                    const GLfloat& nearVal, const GLfloat& farVal)
    {
        GLfloat a = 2 * nearVal / (right - left);
        GLfloat c = (right + left) / (right - left);
        GLfloat f = 2 * nearVal / (top - bottom);
        GLfloat g = (top + bottom) / (top - bottom);
        GLfloat k = -1 * (farVal + nearVal) / (farVal - nearVal);
        GLfloat l = -1 * (2 * farVal * nearVal) / (farVal - nearVal);

        std::vector<std::vector<GLfloat>> v = { { a, 0.0f, c, 0.0f },
                                                { 0.0f, f, g, 0.0f },
                                                { 0.0f, 0.0f, k, l },
                                                { 0.0f, 0.0f, -1.0f, 0.0f } };

        matrix4 M(v);
        return M;
    }

} // namespace mygl

std::ostream& operator<<(std::ostream& out, const mygl::matrix4& m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            out << m.content_[i][j] << " ";
        }
        out << std::endl;
    }

    return out;
}

