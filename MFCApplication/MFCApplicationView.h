#pragma once
#include <vector>
#include <cmath>
#include <random>
#include <gl/GL.h>
#include <gl/GLU.h>

class CMFCApplicationView : public CView
{
protected:
    CMFCApplicationView();
    DECLARE_DYNCREATE(CMFCApplicationView)

public:
    CMFCApplicationDoc* GetDocument() const;

public:
    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
    virtual ~CMFCApplicationView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

private:
    double* mas = nullptr;
    double* sv = nullptr;
    int step;
    bool error;
    int kod_zapol;
    float xRot, yRot, zRot;

    double ctrlPoints[5][5][3];

    HGLRC m_hRC;
    CFont m_font;

    void SetupPixelFormat(CDC* pDC);
    BOOL InitializeOpenGL();
    void Polinom_Surf();
    bool ComputePolynomialCoefficients(double* xData, double* yData, int count, int degree, double* coefficients);
    void RenderText(const wchar_t* text);
    bool GaussianElimination(double* A, double* b, int size, int precision = 0);
    void Udlin_Kontur(double X_kon[], double Y_kon[], double shag_set);
    void Scena(double min_x, double max_x, double min_y, double max_y, int param1, int param2);
    void OutText(const char* text);
};
