
// SimpleRtspClientDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "SimpleRtspClient.h"
#include "SimpleRtspClientDlg.h"
#include "afxdialogex.h"
#include "FFmpegHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSimpleRtspClientDlg 대화 상자



CSimpleRtspClientDlg::CSimpleRtspClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIMPLERTSPCLIENT_DIALOG, pParent)
	, m_ffmpegRtspClient(nullptr)
	, m_videoRenderer(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSimpleRtspClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSimpleRtspClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CSimpleRtspClientDlg 메시지 처리기

BOOL CSimpleRtspClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_ffmpegRtspClient = new FFmpegRtspClient;
	m_ffmpegRtspClient->SetFrameReceiveCallback(this, ReceiveDecodedVideoFrame, nullptr);
	m_ffmpegRtspClient->Open("rtsp://127.0.0.1:8554/test", true);

	AVHWDeviceType HWtype;
	FFmpegHelper::ConfigureHWDecoder(&HWtype);

	auto srcFormat = HWtype == AV_HWDEVICE_TYPE_NONE
		? m_ffmpegRtspClient->GetVideoPixelFormat()
		: FFmpegHelper::GetHWPixelFormat(HWtype);

	CRect rect;
	GetClientRect(rect);

	m_videoFrameConverter = new FFmpegVideoFrameConverter;
	m_videoFrameConverter->Create(m_ffmpegRtspClient->GetVideoWidth(),
		m_ffmpegRtspClient->GetVideoHeight(),
		srcFormat,
		rect.Width(),
		rect.Height(),
		AV_PIX_FMT_YUYV422);

	m_videoRenderer = new SDL2VideoRenderer();
	m_videoRenderer->Create(GetSafeHwnd(),
		m_ffmpegRtspClient->GetVideoWidth(), m_ffmpegRtspClient->GetVideoHeight());

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CSimpleRtspClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSimpleRtspClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CSimpleRtspClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSimpleRtspClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_ffmpegRtspClient)
	{
		delete m_ffmpegRtspClient;
		m_ffmpegRtspClient = nullptr;
	}

	if (m_videoFrameConverter)
	{
		delete m_videoFrameConverter;
		m_videoFrameConverter = nullptr;
	}

	if (m_videoRenderer)
	{
		delete m_videoRenderer;
		m_videoRenderer = nullptr;
	}
}

void CSimpleRtspClientDlg::ReceiveDecodedVideoFrame(void* clientData, int width, int height, AVFrame* frame)
{
	CSimpleRtspClientDlg* object = (CSimpleRtspClientDlg*)clientData;
	if (!object->m_videoFrameConverter || !object->m_videoRenderer) return;

	auto convertedFrame = object->m_videoFrameConverter->Convert(frame);
	object->m_videoRenderer->Update(convertedFrame->data[0], convertedFrame->linesize[0]);
	object->m_videoRenderer->Present();
}