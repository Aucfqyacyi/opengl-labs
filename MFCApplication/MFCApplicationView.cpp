#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "MFCApplication.h"
#endif

#include "MFCApplicationDoc.h"
#include "MFCApplicationView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "MainFrm.h"

IMPLEMENT_DYNCREATE(CMFCApplicationView, CView)

BEGIN_MESSAGE_MAP(CMFCApplicationView, CView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CMFCApplicationView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
    case VK_UP:
        xRot += 5.0f;
        break;
    case VK_DOWN:
        xRot -= 5.0f;
        break;
    case VK_LEFT:
        yRot += 5.0f;
        break;
    case VK_RIGHT:
        yRot -= 5.0f;
        break;
    case 'Q':
        zRot += 5.0f;
        break;
    case 'E':
        zRot -= 5.0f;
        break;
    default:
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
        return;
    }

    Invalidate();
}

CMFCApplicationView::CMFCApplicationView()
    : m_hRC(NULL), step(5), error(false), kod_zapol(1), xRot(0.0f), yRot(0.0f), zRot(0.0f)
{
    double tempCtrlPoints[5][5][3] =
    {
        { { -5.0, -7.0, 2.0 }, { -3.0, -7.0, 2.0 }, { 1.0, -7.0, 2.0 }, { 5.0, -7.0, 2.0 }, { 9.0, -7.0, 2.0 } },
        { { -5.0, -3.0, 5.0 }, { -3.0, -3.0, 6.0 }, { 1.0, -3.0, 6.0 }, { 5.0, -3.0, 6.0 }, { 9.0, -3.0, 5.0 } },
        { { -5.0,  1.0, 3.0 }, { -3.0,  1.0, 4.0 }, { 1.0,  1.0, 4.0 }, { 5.0,  1.0, 4.0 }, { 9.0,  1.0, 3.0 } },
        { { -5.0,  5.0, 6.0 }, { -3.0,  5.0, 7.0 }, { 1.0,  5.0, 7.0 }, { 5.0,  5.0, 7.0 }, { 9.0,  5.0, 6.0 } },
        { { -5.0,  9.0, 1.0 }, { -3.0,  9.0, 1.0 }, { 1.0,  9.0, 1.0 }, { 5.0,  9.0, 1.0 }, { 9.0,  9.0, 1.0 } }
    };
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            for (int k = 0; k < 3; ++k)
                ctrlPoints[i][j][k] = tempCtrlPoints[i][j][k];
}

CMFCApplicationView::~CMFCApplicationView()
{
    if (mas != nullptr)
    {
        delete[] mas;
        mas = nullptr;
    }
    if (sv != nullptr)
    {
        delete[] sv;
        sv = nullptr;
    }
    if (m_font.GetSafeHandle())
    {
        m_font.DeleteObject();
    }
}

#ifdef _DEBUG
void CMFCApplicationView::AssertValid() const
{
    CView::AssertValid();
}

void CMFCApplicationView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif //_DEBUG

CMFCApplicationDoc* CMFCApplicationView::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCApplicationDoc)));
    return (CMFCApplicationDoc*)m_pDocument;
}

BOOL CMFCApplicationView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    return CView::PreCreateWindow(cs);
}

int CMFCApplicationView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    CDC* pDC = GetDC();
    if (!pDC)
    {
        AfxMessageBox(_T("Не удалось получить DC"));
        return -1;
    }

    SetupPixelFormat(pDC);

    m_hRC = wglCreateContext(pDC->GetSafeHdc());
    if (!m_hRC)
    {
        AfxMessageBox(_T("Не удалось создать контекст OpenGL"));
        ReleaseDC(pDC);
        return -1;
    }

    if (!wglMakeCurrent(pDC->GetSafeHdc(), m_hRC))
    {
        AfxMessageBox(_T("Не удалось сделать контекст текущим"));
        wglDeleteContext(m_hRC);
        m_hRC = NULL;
        ReleaseDC(pDC);
        return -1;
    }

    if (!InitializeOpenGL())
    {
        AfxMessageBox(_T("Не удалось инициализировать OpenGL"));
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(m_hRC);
        m_hRC = NULL;
        ReleaseDC(pDC);
        return -1;
    }

    ReleaseDC(pDC);
    return 0;
}

void CMFCApplicationView::SetupPixelFormat(CDC* pDC)
{
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        24,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        32,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelformat = ChoosePixelFormat(pDC->GetSafeHdc(), &pfd);
    if (pixelformat == 0)
    {
        AfxMessageBox(_T("Не удалось выбрать пиксельный формат"));
        return;
    }

    if (!SetPixelFormat(pDC->GetSafeHdc(), pixelformat, &pfd))
    {
        AfxMessageBox(_T("Не удалось установить пиксельный формат"));
        return;
    }

    ::DescribePixelFormat(pDC->GetSafeHdc(), pixelformat, sizeof(pfd), &pfd);
}

BOOL CMFCApplicationView::InitializeOpenGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    CRect rect;
    GetClientRect(&rect);
    gluPerspective(45.0, (double)rect.Width() / (double)rect.Height(), 1.0, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    m_font.CreatePointFont(50, _T("Arial"));
    SelectObject(wglGetCurrentDC(), m_font.m_hObject);

    if (!wglUseFontOutlinesW(wglGetCurrentDC(), 0, 255, 1000, 0, 0, WGL_FONT_POLYGONS, NULL))
    {
        AfxMessageBox(_T("Не удалось создать контуры шрифта"));
        return FALSE;
    }

    return TRUE;
}

void CMFCApplicationView::OnDestroy()
{
    CView::OnDestroy();

    if (m_hRC)
    {
        CDC* pDC = GetDC();
        if (pDC)
        {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(m_hRC);
            m_hRC = NULL;
            ReleaseDC(pDC);
        }
    }

    if (m_font.GetSafeHandle())
    {
        m_font.DeleteObject();
    }
}

void CMFCApplicationView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    if (cy == 0) cy = 1;

    glViewport(0, 0, cx, cy);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)cx / (double)cy, 1.0, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void CMFCApplicationView::OnDraw(CDC* /*pDC*/)
{
    CMFCApplicationDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    CDC* pDC = GetDC();
    if (!pDC)
        return;

    wglMakeCurrent(pDC->GetSafeHdc(), m_hRC);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    gluLookAt(0.0, 0.0, 2.0,
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0);

    Polinom_Surf();

    SwapBuffers(pDC->GetSafeHdc());

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(pDC);
}

void CMFCApplicationView::Polinom_Surf()
{
    if (mas != nullptr)
    {
        delete[] mas;
        mas = nullptr;
    }
    if (sv != nullptr)
    {
        delete[] sv;
        sv = nullptr;
    }

    mas = new double[21 * 21];
    sv = new double[21];

    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot, 0.0f, 0.0f, 1.0f);
    glRotatef(zRot, 0.0f, 1.0f, 0.0f);

    int i, j, k1, k2, k;
    double Xmax, Xmin, Ymax, Ymin, x, y, z;
    double X_kon[4], Y_kon[4];
    CString sum1 = L"";
    CString sum2 = L"";
    CString sum3 = L"";
    double min_par[3], max_par[3], dxy;
    int c_i, k_i, p_i, c_j, k_j, p_j;
    double MO;
    double shag_set = 0.5;
    int kol = (step + 1) * (step + 2) / 2;
    CString str_pol;

    int kol_str = 5, kol_stol = 5;

    for (i = 0; i < 3; i++)
        max_par[i] = min_par[i] = ctrlPoints[0][0][i];

    MO = 0;

    for (i = 0; i < kol_str; i++)
        for (j = 0; j < kol_stol; j++)
            for (k = 0; k < 3; k++)
            {
                if (ctrlPoints[i][j][k] < min_par[k]) min_par[k] = ctrlPoints[i][j][k];
                if (ctrlPoints[i][j][k] > max_par[k]) max_par[k] = ctrlPoints[i][j][k];
                if (k == 2) MO += ctrlPoints[i][j][2];
            }

    MO /= (1.0 * kol_str * kol_stol);

    dxy = (max_par[0] - min_par[0] > max_par[1] - min_par[1]) ?
        max_par[0] - min_par[0] : max_par[1] - min_par[1];

    X_kon[0] = X_kon[1] = min_par[0];
    X_kon[2] = X_kon[3] = max_par[0];
    Y_kon[0] = Y_kon[3] = min_par[1];
    Y_kon[1] = Y_kon[2] = max_par[1];

    Udlin_Kontur(X_kon, Y_kon, shag_set);

    for (i = 0; i < kol; i++)
    {
        sv[i] = 0;
        c_i = floor((1 + sqrt(1.0 * 8 * (i + 1) - 7)) / 2);
        k_i = c_i * (c_i + 1) / 2 - (i + 1);
        p_i = i - c_i * (c_i - 1) / 2;

        for (j = 0; j < kol; j++)
        {
            mas[i * kol + j] = 0;
            c_j = floor((1 + sqrt(1.0 * 8 * (j + 1) - 7)) / 2);
            k_j = c_j * (c_j + 1) / 2 - (j + 1);
            p_j = j - c_j * (c_j - 1) / 2;

            for (k1 = 0; k1 < kol_str; k1++)
                for (k2 = 0; k2 < kol_stol; k2++)
                {
                    mas[i * kol + j] += pow(ctrlPoints[k1][k2][0], (k_i + k_j)) *
                        pow(ctrlPoints[k1][k2][1], (p_i + p_j));
                    if (j == 0)
                        sv[i] += ctrlPoints[k1][k2][2] * pow(ctrlPoints[k1][k2][0], k_i) *
                        pow(ctrlPoints[k1][k2][1], p_i);
                }
        }
    }

    while (1)
    {
        if (error == true || !GaussianElimination(mas, sv, kol))
        {
            error = true;
            break;
        }

        int kol_x = static_cast<int>((max_par[0] - min_par[0] + shag_set / 2) / shag_set) + 1;
        int kol_y = static_cast<int>((max_par[1] - min_par[1] + shag_set / 2) / shag_set) + 1;

        double* stroka = new double[kol_x];
        double z1;

        glPushMatrix();

        glPointSize(2.0);
        for (int kk = kod_zapol; kk > -1; kk--)
        {
            if (kk == 1)
            {
                glTranslatef(0.02f, 0.02f, 0.02f);
                glColor3f(0.8f, 0.2f, 0.8f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            if (kk == 0)
            {
                if (step < 2)    glTranslatef(0.0f, 0.0f, 0.01f);
                if (step > 1)    glScalef(1.01f, 1.01f, 1.01f);
                glColor3f(1.0f, 1.0f, 1.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }

            glBegin(GL_QUADS);

            for (i = 0; i < kol_y; i++)
            {
                y = min_par[1] + i * shag_set;

                for (j = 0; j < kol_x; j++)
                {
                    x = min_par[0] + j * shag_set;

                    z = sv[0];
                    if (step > 0) z += sv[1] * x + sv[2] * y;
                    if (step > 1) z += sv[3] * pow(x, 2) + sv[4] * x * y + sv[5] * pow(y, 2);
                    if (step > 2) z += sv[6] * pow(x, 3) + sv[7] * pow(x, 2) * y + sv[8] * x * pow(y, 2) + sv[9] * pow(y, 3);
                    if (step > 3) z += sv[10] * pow(x, 4) + sv[11] * pow(x, 3) * y + sv[12] * pow(x, 2) * pow(y, 2) +
                        sv[13] * x * pow(y, 3) + sv[14] * pow(y, 4);
                    if (step > 4) z += sv[15] * pow(x, 5) + sv[16] * pow(x, 4) * y + sv[17] * pow(x, 3) * pow(y, 2) +
                        sv[18] * pow(x, 2) * pow(y, 3) + sv[19] * x * pow(y, 4) + sv[20] * pow(y, 5);

                    z = (z - min_par[2]) / (max_par[2] - min_par[2]) - 0.5;

                    if (i == 0)
                    {
                        stroka[j] = z;
                        continue;
                    }

                    if (j == 0)
                    {
                        z1 = z;
                        continue;
                    }

                    double Xmin = ((j - 1) * shag_set) / dxy - 0.5;
                    double Xmax = (j * shag_set) / dxy - 0.5;
                    double Ymin = ((i - 1) * shag_set) / dxy - 0.5;
                    double Ymax = (i * shag_set) / dxy - 0.5;

                    glVertex3f(static_cast<float>(Xmin), static_cast<float>(Ymin), static_cast<float>(stroka[j - 1]));
                    glVertex3f(static_cast<float>(Xmin), static_cast<float>(Ymax), static_cast<float>(z1));
                    glVertex3f(static_cast<float>(Xmax), static_cast<float>(Ymax), static_cast<float>(z));
                    glVertex3f(static_cast<float>(Xmax), static_cast<float>(Ymin), static_cast<float>(stroka[j]));

                    stroka[j - 1] = z1;
                    z1 = z;

                    if (j == kol_x - 1) stroka[j] = z;
                }
            }

            glEnd();
        }

        sum1 = L" ";
        sum2 = L" ";
        sum3 = L" ";

        for (i = 0; i < kol; i++)
        {
            if (i == 0) str_pol.Format(L" z = %3.3f  ", sv[i]);
            if (i == 1) str_pol.Format(L" %3.3fx  ", sv[i]);
            if (i == 2) str_pol.Format(L" %3.3fy  ", sv[i]);
            if (i == 3) str_pol.Format(L" %3.3fxy  ", sv[i]);
            if (i == 4) str_pol.Format(L" %3.3fx^2  ", sv[i]);
            if (i == 5) str_pol.Format(L" %3.3fy^2  ", sv[i]);

            if (i == 6) str_pol.Format(L" %3.3fx^3  ", sv[i]);
            if (i == 7) str_pol.Format(L" %3.3fx^2y  ", sv[i]);
            if (i == 8) str_pol.Format(L" %3.3fxy^2  ", sv[i]);
            if (i == 9) str_pol.Format(L" %3.3fy^3  ", sv[i]);

            if (i == 10) str_pol.Format(L" %3.3fx^4  ", sv[i]);
            if (i == 11) str_pol.Format(L" %3.3fx^3y  ", sv[i]);
            if (i == 12) str_pol.Format(L" %3.3fx^2y^2  ", sv[i]);
            if (i == 13) str_pol.Format(L" %3.3fxy^3  ", sv[i]);
            if (i == 14) str_pol.Format(L" %3.3fy^4  ", sv[i]);

            if (i == 15) str_pol.Format(L" %3.3fx^5  ", sv[i]);
            if (i == 16) str_pol.Format(L" %3.3fx^4y  ", sv[i]);
            if (i == 17) str_pol.Format(L" %3.3fx^3y^2  ", sv[i]);
            if (i == 18) str_pol.Format(L" %3.3fx^2y^3  ", sv[i]);
            if (i == 19) str_pol.Format(L" %3.3fxy^4  ", sv[i]);
            if (i == 20) str_pol.Format(L" %3.3fy^5  ", sv[i]);
            if (i > 20) break;

            if (i < 9) sum1 += str_pol;
            if (i > 8 && i < 17) sum2 += str_pol;
            if (i > 16) sum3 += str_pol;
        }

        delete[] stroka;
        glPopMatrix();

        break;
    }

    glColor3ub(0, 0, 0);
    glPointSize(8.0f);
    glBegin(GL_POINTS);

    for (i = 0; i < kol_str; i++)
        for (j = 0; j < kol_stol; j++)
        {
            x = (ctrlPoints[i][j][0] - min_par[0]) / dxy - 0.5;
            y = (ctrlPoints[i][j][1] - min_par[1]) / dxy - 0.5;
            z = (ctrlPoints[i][j][2] - min_par[2]) / (max_par[2] - min_par[2]) - 0.5;
            glVertex3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
        }
    glEnd();

    Scena(min_par[0], max_par[0], min_par[1], max_par[1], 1, 8);

    char buf[400];
    size_t dd;
    glColor3ub(0, 0, 0);
    glLoadIdentity();

    if (error == false)
    {
        glTranslatef(-0.5f, -0.95f, 0.0f);
        glScalef(0.15f, 0.15f, 0.15f);
        sprintf_s(buf, "Степень полинома %d", step);
    }
    if (error == true)
    {
        glTranslatef(-1.5f, 1.0f, 0.0f);
        glScalef(0.15f, 0.15f, 0.15f);
        sprintf_s(buf, "Ошибка выполнения вычислений полинома");
        OutText(buf);
        return;
    }

    OutText(buf);
    glLoadIdentity();
    glTranslatef(-1.6f, 1.1f, 0.0f);
    glScalef(0.09f, 0.09f, 0.09f);
    wcstombs_s(&dd, buf, sum1.GetLength() + 1, sum1.GetBuffer(), _TRUNCATE);
    OutText(buf);

    glLoadIdentity();
    glTranslatef(-1.6f, 1.0f, 0.0f);
    glScalef(0.09f, 0.09f, 0.09f);
    wcstombs_s(&dd, buf, sum2.GetLength() + 1, sum2.GetBuffer(), _TRUNCATE);
    OutText(buf);

    glLoadIdentity();
    glTranslatef(-1.6f, 0.9f, 0.0f);
    glScalef(0.09f, 0.09f, 0.09f);
    wcstombs_s(&dd, buf, sum3.GetLength() + 1, sum3.GetBuffer(), _TRUNCATE);
    OutText(buf);
}

void CMFCApplicationView::Udlin_Kontur(double X_kon[], double Y_kon[], double shag_set)
{
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(static_cast<float>(X_kon[0]), static_cast<float>(Y_kon[0]), 0.0f);
    glVertex3f(static_cast<float>(X_kon[1]), static_cast<float>(Y_kon[1]), 0.0f);
    glVertex3f(static_cast<float>(X_kon[2]), static_cast<float>(Y_kon[2]), 0.0f);
    glVertex3f(static_cast<float>(X_kon[3]), static_cast<float>(Y_kon[3]), 0.0f);
    glEnd();
}

void CMFCApplicationView::Scena(double min_x, double max_x, double min_y, double max_y, int param1, int param2)
{
    glLineWidth(2.0f);
    glColor3ub(0, 0, 0);

    glBegin(GL_LINES);
    glVertex3f(static_cast<float>(min_x), 0.0f, 0.0f);
    glVertex3f(static_cast<float>(max_x), 0.0f, 0.0f);
    glVertex3f(0.0f, static_cast<float>(min_y), 0.0f);
    glVertex3f(0.0f, static_cast<float>(max_y), 0.0f);
    glVertex3f(0.0f, 0.0f, static_cast<float>(min_y));
    glVertex3f(0.0f, 0.0f, static_cast<float>(max_y));
    glEnd();
}

void CMFCApplicationView::OutText(const char* text)
{
    wchar_t wtext[400];
    mbstowcs_s(NULL, wtext, text, 400);
    RenderText(wtext);
}

void CMFCApplicationView::RenderText(const wchar_t* text)
{
    glListBase(1000);
    glCallLists(static_cast<GLsizei>(wcslen(text)), GL_UNSIGNED_SHORT, text);
}

bool CMFCApplicationView::GaussianElimination(double* A, double* b, int size, int precision)
{
    int pivotRow = 0;
    double scaleFactor = 0.0;

    if (precision == 0)
        scaleFactor = 1.0;
    else
        scaleFactor = pow(10.0, precision);

    try
    {
        for (int i = 0; i < size; ++i)
        {
            double maxElement = 0.0;
            int maxRow = i;
            for (int j = i; j < size; ++j)
            {
                if (fabs(A[j * size + i]) > fabs(maxElement))
                {
                    maxElement = A[j * size + i];
                    maxRow = j;
                }
            }

            for (int j = i; j < size; ++j)
            {
                std::swap(A[i * size + j], A[maxRow * size + j]);
            }
            std::swap(b[i], b[maxRow]);

            double pivot = A[i * size + i];
            if (pivot == 0.0)
                return false;

            for (int j = i; j < size; ++j)
            {
                A[i * size + j] /= pivot;
            }
            b[i] /= pivot;

            for (int k = 0; k < size; ++k)
            {
                if (k == i) continue;
                double factor = A[k * size + i];
                for (int j = i; j < size; ++j)
                {
                    A[k * size + j] -= factor * A[i * size + j];
                }
                b[k] -= factor * b[i];
            }
        }

        for (int i = 0; i < size; ++i)
        {
            if (isnan(b[i]) || isinf(b[i]))
                return false;
            if (precision > 0 && fabs(b[i]) > scaleFactor)
                return false;
        }

        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool CMFCApplicationView::ComputePolynomialCoefficients(double* xData, double* yData, int count, int degree, double* coefficients)
{
    if (count < degree + 1) return false;

    int matrixSize = degree + 1;
    std::vector<double> matrix(matrixSize * matrixSize, 0.0);

    for (int row = 0; row < matrixSize; ++row)
    {
        coefficients[row] = 0.0;
        for (int col = 0; col < matrixSize; ++col)
        {
            for (int k = 0; k < count; ++k)
            {
                matrix[row * matrixSize + col] += pow(xData[k], row + col);
                if (col == 0)
                {
                    coefficients[row] += pow(xData[k], row) * yData[k];
                }
            }
        }
    }

    bool success = GaussianElimination(matrix.data(), coefficients, matrixSize);
    return success;
}
