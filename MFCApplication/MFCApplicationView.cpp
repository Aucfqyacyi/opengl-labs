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
END_MESSAGE_MAP()

CMFCApplicationView::CMFCApplicationView()
    : m_hRC(NULL)
{
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
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
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
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Белый фон

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

    // Используем Unicode версию функции
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
    CDC* pDC = GetDC();
    if (!pDC)
        return;

    wglMakeCurrent(pDC->GetSafeHdc(), m_hRC);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    gluLookAt(0.0, 0.0, 35.0,
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0);

    DrawPolynomial();

    SwapBuffers(pDC->GetSafeHdc());

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(pDC);
}

void CMFCApplicationView::DrawPolynomial()
{
    const int dataCount = 11;
    const int polynomialDegree = 6;
    double xValues[] = { -9, -6, -4, -2, 0, 2, 3, 5, 7, 10, 12 };
    double yValues[] = { 1, 4, 3, 6, 9, 5, 7, 8, 4, 11, 2 };



    double coefficients[7] = { 0 };

    if (!ComputePolynomialCoefficients(xValues, yValues, dataCount, polynomialDegree, coefficients))
        return;

    // Установка цвета полинома (красный)
    glColor3f(1.0f, 0.0f, 0.0f);
    GLushort dashPattern = 0x5555;

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(5, dashPattern);
    glLineWidth(5.0f);
    glBegin(GL_LINE_STRIP);

    for (double currentX = -9.5; currentX < 9.45; currentX += 0.1)
    {
        double currentY = 0.0;
        for (int power = 0; power <= polynomialDegree; ++power)
        {
            currentY += coefficients[power] * pow(currentX, power);
        }
        glVertex2f(static_cast<float>(currentX), static_cast<float>(currentY));
    }
    glEnd();

    glDisable(GL_LINE_STIPPLE);

    // Установка цвета точек (голубой)
    glPointSize(15.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    for (int idx = 0; idx < dataCount; ++idx)
    {
        glVertex2f(static_cast<float>(xValues[idx]), static_cast<float>(yValues[idx]));
    }
    glEnd();

    // Соединение точек линией (чёрный)
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_STRIP);
    for (int idx = 0; idx < dataCount; ++idx)
    {
        glVertex2f(static_cast<float>(xValues[idx]), static_cast<float>(yValues[idx]));
    }
    glEnd();

    // Рисование осей координат
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    // Ось X
    glVertex2f(-10.0f, 0.0f);
    glVertex2f(10.0f, 0.0f);

    // Ось Y
    glVertex2f(0.0f, -4.0f);
    glVertex2f(0.0f, 10.0f);
    glEnd();

    // Установка цвета текста (чёрный)
    glColor3f(0.0f, 0.0f, 0.0f);

    glPushMatrix();

    // Отрисовка надписей
    glLoadIdentity();
    glTranslatef(-6.0f, -6.0f, 0.0f);
    wchar_t labelBuffer[70];

    glLoadIdentity();
    glTranslatef(9.3f, 0.5f, 0.0f);

    glLoadIdentity();
    glTranslatef(0.2f, 0.4f, 0.0f);

    glLoadIdentity();
    glTranslatef(0.2f, 9.0f, 0.0f);

    glPopMatrix();
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
