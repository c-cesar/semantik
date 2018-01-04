// Thomas Nagy 2013-2018 GPLV3

#ifndef DIAGRAM_WIDGET_H
#define DIAGRAM_WIDGET_H

#include "con.h"

#include <QFrame>

#include <QUrl>

class box_view;
class sem_mediator;
class diagram_document : public QFrame
{
	Q_OBJECT

	public:
		diagram_document(QWidget *i_oParent=0);
		virtual ~diagram_document();
		void init();

		box_view *m_oDiagramView;
		sem_mediator *m_oMediator;

	public slots:
		void slot_open();
		void slot_tab_name(const QUrl&);

	signals:
		void sig_tab_name(diagram_document*, const QUrl&);
};

#endif

