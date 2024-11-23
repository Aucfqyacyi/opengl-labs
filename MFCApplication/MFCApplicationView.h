
// MFCApplicationView.h : interface of the CMFCApplicationView class
//
#pragma once
#include <vector>
#include <cmath>

class CMFCApplicationView : public CView
{
	bool ComputePolynomialCoefficients(double* x, double* y, int n, int st, double* b);
	bool GaussianElimination(double* A, double* LL, int n, int err = 0);
	void DrawPolynomial();
protected:
    CMFCApplicationView();

public:
    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
    HGLRC m_hRC;           // Контекст OpenGL
    CFont m_font;          // Шрифт для OpenGL
    BOOL InitializeOpenGL(); // Метод инициализации OpenGL
    void SetupPixelFormat(CDC* pDC);

    // Очистка ресурсов
    virtual void OnDestroy();

    DECLARE_MESSAGE_MAP()
    DECLARE_DYNCREATE(CMFCApplicationView)
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in MFCApplicationView.cpp
inline CMFCApplicationDoc* CMFCApplicationView::GetDocument() const
   { return reinterpret_cast<CMFCApplicationDoc*>(m_pDocument); }
#endif

