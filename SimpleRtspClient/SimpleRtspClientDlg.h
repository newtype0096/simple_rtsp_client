
// SimpleRtspClientDlg.h: 헤더 파일
//

#pragma once
#include "SDL2VideoRenderer.h"
#include "FFmpegRtspClient.h"
#include "FFmpegVideoFrameConverter.h"

// CSimpleRtspClientDlg 대화 상자
class CSimpleRtspClientDlg : public CDialogEx
{
	// 생성입니다.
public:
	CSimpleRtspClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIMPLERTSPCLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	FFmpegRtspClient* m_ffmpegRtspClient;
	FFmpegVideoFrameConverter* m_videoFrameConverter;

	SDL2VideoRenderer* m_videoRenderer;

	static void ReceiveDecodedVideoFrame(void* clientData, int width, int height, AVFrame* frame);
};
