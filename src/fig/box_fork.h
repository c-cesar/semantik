// Thomas Nagy 2007-2012 GPLV3

#ifndef BOX_FORK_H
#define BOX_FORK_H

#include <QGraphicsRectItem>
#include <QBrush>
#include "CON.h"
#include "con.h"

#define OFF 3
#define FORK_LENGTH 160
#define FORK_WIDTH 8

class QTextDocument;
class box_view;
class data_item;
class data_box;
class box_fork : public QGraphicsRectItem, public connectable
{
	public:
		box_fork(box_view*, int i_iId);
		~box_fork();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);

		QRectF rect() const { return boundingRect().translated(pos()).adjusted(JUST, JUST, -JUST, -JUST); };

		box_view *m_oView;
		data_item *m_oItem;

		void mousePressEvent(QGraphicsSceneMouseEvent* e);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);

		void update_data();
		void update_links();

		QVariant itemChange(GraphicsItemChange i_oChange, const QVariant &i_oValue);
		virtual int choose_position(const QPointF&p, int id=-1);
};

#endif // BOX_FORK_H

