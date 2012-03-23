#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <afxwin.h>
#include <afxtempl.h>
#include "consts.h"
#include "DIB.h"

enum NotificationPosition
{
	NotificationPositionLeftTop = 0,
	NotificationPositionTopMiddle,
	NotificationPositionRightTop,
	NotificationPositionRightMiddle,
	NotificationPositionRightBottom,
	NotificationPositionBottomMiddle,
	NotificationPositionLeftBottom,
	NotificationPositionLeftMiddle
};
typedef enum NotificationPosition NotificationPosition;

class CNotification: public CFrameWnd
{
public:
	CNotification();
	~CNotification();

	void LayerDraw(CDIB *dib = NULL);
	void LayerUpdate(CDIB *dib = NULL);

	int CalculateWidth(int defaultSize);

public:
	CDIB *dib;
	NotificationPosition position;

public:
	afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);
	afx_msg void OnClose();
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP();
};

#endif /* NOTIFICATIONS_H */