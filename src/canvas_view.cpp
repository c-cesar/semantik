// Thomas Nagy 2007-2009 GPLV3

#include "canvas_flag.h"
#include "data_item.h"
#include  <QtGlobal>
#include "data_control.h"
#include "canvas_view.h"
#include  <QMenu> 
#include <QToolTip>
#include  <QColorDialog> 
#include<KToolBar> 
#include  <QActionGroup> 
#include "canvas_item.h"
#include "canvas_link.h"
#include<QCoreApplication>
#include "canvas_sort.h"
#include "semantik.h"
#include <QTextCursor> 
#include  <QApplication> 
#include <QtDebug>
#include  <QX11Info>
#include <QScrollBar>
#include  <QGraphicsScene>
#include <QWheelEvent>
#include  <QMatrix>
#include <QPointF>

#include <math.h>

canvas_view::canvas_view(QWidget *i_oWidget, data_control *i_oControl) : QGraphicsView(i_oWidget)
{
	m_oSemantikWindow = i_oWidget;

	m_oRubbery = new QRubberBand(QRubberBand::Rectangle, this);
	m_oRubbery->setGeometry(QRect(0, 0, 0, 0));

	QGraphicsScene *l_oScene = new QGraphicsScene(this);
	l_oScene->setSceneRect(-400, -400, 400, 400);
	setScene(l_oScene);

	//setCacheMode(CacheBackground);
	setRenderHint(QPainter::Antialiasing);

	// another bait !!
	#ifndef Q_OS_DARWIN

	setDragMode(QGraphicsView::NoDrag);

	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setMinimumSize(200, 200);

	#ifdef O
	//setTransformationAnchor(AnchorUnderMouse);
	//setResizeAnchor(AnchorViewCenter);
	#endif

	/*QMatrix k(0, 1, 1, 0, 0, 0);
	QMatrix kv(1, 0, 0, -1, 0, 0);*/
	m_oControl = i_oControl;
	/*setMatrix(l_oMatrix);
	scale(0.8, 0.8);*/
	#endif

	m_iMode = select_mode;
	m_iLastMode = no_mode;

	m_bPressed = false;

	m_oRubberLine = new rubber_line(QRubberBand::Line, this);


	QBrush l_oBrush = QBrush();
	//QColor l_oColor = QColor("#000077");
	//l_oColor.setAlpha(200);

	QColor l_oColor = QColor(0, 0, 150, 100);
	//l_oBrush.setColor(l_oColor);
	//m_oRubber->setBrush(l_oBrush);

	QAction *l_o = NULL;

	m_oEditAction = new QAction(trUtf8("Toggle edit"), this);
	m_oEditAction->setShortcut(notr("Return"));
        connect(m_oEditAction, SIGNAL(triggered()), this, SLOT(slot_edit()));
	addAction(m_oEditAction);

	m_oAddItemAction = new QAction(trUtf8("Insert child"), this);
	m_oAddItemAction->setShortcut(notr("Ctrl+Return"));
        connect(m_oAddItemAction, SIGNAL(triggered()), this, SLOT(slot_add_item()));
	addAction(m_oAddItemAction);

	m_oDeleteAction = new QAction(trUtf8("Delete selection"), this);
	m_oDeleteAction->setShortcut(notr("Del"));
        connect(m_oDeleteAction, SIGNAL(triggered()), this, SLOT(slot_delete()));
	addAction(m_oDeleteAction);

	l_o = new QAction(trUtf8("Insert sibling"), this);
	l_o->setShortcut(notr("Shift+Return"));
        connect(l_o, SIGNAL(triggered()), this, SLOT(slot_add_sibling()));
	addAction(l_o);

	l_o = new QAction(trUtf8("Move up"), this); l_o->setShortcut(notr("Alt+Up")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_move())); addAction(l_o); l_o->setData(QVariant(0));
	l_o = new QAction(trUtf8("Move down"), this); l_o->setShortcut(notr("Alt+Down")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_move())); addAction(l_o); l_o->setData(QVariant(1));
	l_o = new QAction(trUtf8("Move left"), this); l_o->setShortcut(notr("Alt+Left")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_move())); addAction(l_o); l_o->setData(QVariant(2));
	l_o = new QAction(trUtf8("Move right"), this); l_o->setShortcut(notr("Alt+Right")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_move())); addAction(l_o); l_o->setData(QVariant(3));

	l_o = new QAction(trUtf8("Select up"), this); l_o->setShortcut(notr("Up")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_sel())); addAction(l_o); l_o->setData(QVariant(0));
	l_o = new QAction(trUtf8("Select down"), this); l_o->setShortcut(notr("Down")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_sel())); addAction(l_o); l_o->setData(QVariant(1));
	l_o = new QAction(trUtf8("Select left"), this); l_o->setShortcut(notr("Left")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_sel())); addAction(l_o); l_o->setData(QVariant(2));
	l_o = new QAction(trUtf8("Select right"), this); l_o->setShortcut(notr("Right")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_sel())); addAction(l_o); l_o->setData(QVariant(3));


	l_o = new QAction(trUtf8("Next root"), this); l_o->setShortcut(notr("PgDown")); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_hop())); addAction(l_o); l_o->setData(QVariant(1));


	m_oMenu = new QMenu(this);
	m_oMenu->addAction(m_oAddItemAction);
	m_oMenu->addAction(m_oEditAction);
	m_oMenu->addAction(m_oDeleteAction);
	m_oColorMenu = m_oMenu->addMenu(trUtf8("Colors"));
	m_oDataMenu = m_oMenu->addMenu(trUtf8("Data type"));

	l_o = new QAction(trUtf8("Text"), this); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_change_data())); addAction(l_o); l_o->setData(QVariant(view_text));
	m_oDataMenu->addAction(l_o);
	l_o = new QAction(trUtf8("Diagram"), this); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_change_data())); addAction(l_o); l_o->setData(QVariant(view_diag));
	m_oDataMenu->addAction(l_o);
	l_o = new QAction(trUtf8("Table"), this); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_change_data())); addAction(l_o); l_o->setData(QVariant(view_table));
	m_oDataMenu->addAction(l_o);
	l_o = new QAction(trUtf8("Image"), this); connect(l_o, SIGNAL(triggered()), this, SLOT(slot_change_data())); addAction(l_o); l_o->setData(QVariant(view_img));
	m_oDataMenu->addAction(l_o);
}

void canvas_view::slot_hop()
{
	//qDebug()<<"hop";
	switch (((QAction*) QObject::sender())->data().toInt())
	{
		case 0:
			if (m_oSelected.size()>1) deselect_all();
			m_oControl->prev_root();
			break;
		case 1:
			if (m_oSelected.size()>1) deselect_all();
			m_oControl->next_root();
			break;
		default:
			break;
	}
	if (m_oSelected.size()==1) ensureVisible(m_oSelected[0]);
}

void canvas_view::slot_edit()
{
	if (m_oSelected.size() == 1)
	{
		if (m_oSelected[0]->m_bEdit)
		{
			QFocusEvent l_oEv = QFocusEvent(QEvent::FocusOut);
			m_oSelected[0]->focus_out(&l_oEv);
			m_oSelected[0]->update();
		}
		else
		{
			m_oSelected[0]->focus_in();
			m_oSelected[0]->update();
		}
	}
}

void canvas_view::slot_move()
{
	if (m_oSelected.size() < 1) return;
	switch (((QAction*) QObject::sender())->data().toInt())
	{
		case 0: move_sel(0, -20); break;
		case 1: move_sel(0, 20); break;
		case 2: move_sel(-20, 0); break;
		case 3: move_sel(20, 0); break;
		default: break;
	}
}

void canvas_view::slot_sel()
{
	if (m_oSelected.size() != 1) return;
	int l_iId = m_oSelected[0]->Id();
	switch (((QAction*) QObject::sender())->data().toInt())
	{
		case 0: m_oControl->select_item_keyboard(l_iId, 3); break;
		case 1: m_oControl->select_item_keyboard(l_iId, 4); break;
		case 2: m_oControl->select_item_keyboard(l_iId, 1); break;
		case 3: m_oControl->select_item_keyboard(l_iId, 2); break;
		default: break;
	}
	if (m_oSelected.size()==1) ensureVisible(m_oSelected[0]);
}

void canvas_view::slot_add_item()
{
	//if (m_oSelected.size() != 1) return;
	int l_iId = 0;
	if (m_oSelected.size()==1) l_iId = m_oSelected[0]->Id();
	deselect_all();

	m_oControl->add_item(l_iId);
	m_oControl->do_reorganize();
	if (m_oSelected.size() == 1) m_oSelected[0]->focus_in();
}

void canvas_view::slot_add_sibling()
{
	if (m_oSelected.size() != 1) return;
	int l_iId = m_oControl->parent_of(m_oSelected[0]->Id());
	if (l_iId == NO_ITEM) return;
	deselect_all();

	m_oControl->add_item(l_iId);
	m_oControl->do_reorganize();
	if (m_oSelected.size() == 1) m_oSelected[0]->focus_in();
}

void canvas_view::slot_delete()
{
	QList<int> l_oLst;
	foreach (QGraphicsItem *l_oItem, m_oSelected)
	{
		if (l_oItem->type() != gratype(7)) continue;
		canvas_item *l_oR = (canvas_item*) l_oItem;
		l_oLst.push_back(l_oR->Id());
	}
	foreach (int i_i, l_oLst)
	{
		m_oControl->remove_item(i_i);
	}
	m_oControl->select_item(NO_ITEM);
}

void canvas_view::show_sort(int i_iId, bool i_b)
{
	int j=0;
	for (int i=0; i<m_oControl->m_oLinks.size(); i++)
	{
		QPoint l_oP = m_oControl->m_oLinks.at(i);
		if (l_oP.x() == i_iId)
		{
			++j;
			canvas_item *l_oRect = m_oItems.value(l_oP.y());

			QString l_sNum = QString::number(j);
			if (l_sNum != l_oRect->m_sNum)
			{
				l_oRect->m_sNum = l_sNum;
				l_oRect->m_oSort->update();
			}
			l_oRect->m_oSort->setVisible(i_b);
		}
	}
}

void canvas_view::move_sel(int i_iX, int i_iY)
{
	check_canvas_size();
	foreach (canvas_item *l_oItem, m_oSelected) { l_oItem->moveBy(i_iX, i_iY); }
	foreach (canvas_item *l_oItem, m_oSelected) { l_oItem->update_links(); ensureVisible(l_oItem); }
}

void canvas_view::set_mode(mode_type i_iMode)
{
	deselect_all();

	switch (i_iMode)
	{
		case select_mode:
			viewport()->setCursor(Qt::ArrowCursor);
			break;
		case link_mode:
			viewport()->setCursor(Qt::CrossCursor);
			break;
		case sort_mode:
			viewport()->setCursor(Qt::ArrowCursor);
			break;
		case scroll_mode:
			viewport()->setCursor(Qt::OpenHandCursor);
			break;
		default:
			break;
			qDebug()<<"unknown mode"<<i_iMode;
	}
	m_iMode = i_iMode;

	// reset
	m_bPressed = false;
	// unneeded statements
	m_oRubbery->hide();
	m_oRubberLine->hide();
}

void canvas_view::keyReleaseEvent(QKeyEvent *i_oEvent)
{
#if 0
	if (m_oSelected.size() == 1)
        {
                m_oSelected[0]->keyReleaseEvent(i_oEvent);
                return;
        }
	QApplication::sendEvent(scene(), i_oEvent);
#endif
	i_oEvent->accept();
}

void canvas_view::keyPressEvent(QKeyEvent *i_oEvent)
{
	if (m_oSelected.size() == 1)
	{
		canvas_item * l_oItem = m_oSelected[0];
		if (l_oItem->m_bEdit && i_oEvent->type() == QEvent::KeyPress)
		{
			l_oItem->keyPressEvent(i_oEvent);
		}
	}
	i_oEvent->accept();
}

void canvas_view::zoom_in()
{
	double i_iScaleFactor = 1.05;
	qreal i_rFactor = matrix().scale(i_iScaleFactor, i_iScaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (i_rFactor < 0.01 || i_rFactor > 1000) return;
	scale(i_iScaleFactor, i_iScaleFactor);
}

void canvas_view::zoom_out()
{
	double i_iScaleFactor = 0.95;
	qreal i_rFactor = matrix().scale(i_iScaleFactor, i_iScaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (i_rFactor < 0.01 || i_rFactor > 1000) return;
	scale(i_iScaleFactor, i_iScaleFactor);
}

void canvas_view::wheelEvent(QWheelEvent *i_oEvent)
{
	QPointF l_o = mapToScene(i_oEvent->pos());
	qreal i_iScaleFactor = pow(2.0, i_oEvent->delta() / 440.0);
	qreal i_rFactor = matrix().scale(i_iScaleFactor, i_iScaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (i_rFactor < 0.01 || i_rFactor > 1000) return;
	scale(i_iScaleFactor, i_iScaleFactor);
	centerOn(l_o + mapToScene(viewport()->rect().center()) - mapToScene(i_oEvent->pos()));
}

bool canvas_view::event(QEvent *i_oEvent)
{
	//qDebug()<<"event type "<<i_oEvent->type();
	if (i_oEvent->type() == QEvent::ToolTip)
	{
		QHelpEvent *l_oEv = static_cast<QHelpEvent*>(i_oEvent);
		QGraphicsItem *l_oItem = scene()->itemAt(mapToScene(l_oEv->pos()));
		if (l_oItem && l_oItem->type() == gratype(7))
		{
			data_item *l_o = *m_oControl + ((canvas_item*) l_oItem)->Id();
			if (l_o && l_o->m_iTextLength > 1)
			{
				QToolTip::showText(l_oEv->globalPos(), l_o->m_sText);
			}
		}
	}
	else if (i_oEvent->type() == QEvent::InputMethod)
	{
		if (m_oSelected.size() == 1)
		{
			canvas_item * l_oItem = m_oSelected[0];
			if (l_oItem->m_bEdit)
			{
				l_oItem->inputMethodEvent((QInputMethodEvent*) i_oEvent);
			}
		}
	}
	else if (i_oEvent->type() == QEvent::KeyPress
		|| i_oEvent->type() == QEvent::ShortcutOverride
		|| i_oEvent->type() == QEvent::Shortcut)
	{
		if (m_oSelected.size() == 1 && m_oSelected[0]->m_bEdit)
		{
			QKeyEvent *l_o = (QKeyEvent*) i_oEvent;
			if (m_oEditAction->shortcut().matches(l_o->key()))
			{
				//qDebug()<<"matches"<<l_o->key();
				return QGraphicsView::event(i_oEvent);
			}

			keyPressEvent(l_o);
			return true;
		}
	}
	return QGraphicsView::event(i_oEvent);
}

void canvas_view::focusOutEvent(QFocusEvent *i_oEv)
{
	foreach (canvas_item *l_oItem, m_oSelected)
        {
		l_oItem->focus_out(i_oEv);
        }
	foreach (QAction* l_o, actions())
	{
		l_o->setEnabled(false);
	}
	if (m_oMenu->isVisible()) enable_menu_actions();
}

/*!
 * eow this actually works
 */
void canvas_view::synchro_doc(const hash_params& i_o)
{
	int l_iCmd = i_o[data_commande].toInt();
	switch (l_iCmd)
	{
		case cmd_add_item:
			{
				int l_iId = i_o[data_id].toInt();
				Q_ASSERT(! m_oItems.contains(l_iId));

				canvas_item*l_oR = new canvas_item(this, l_iId);

				l_oR->setSelected(true);

				QRectF l_oRect = l_oR->boundingRect();
				l_oR->setPos(m_oLastPoint - QPointF(l_oRect.width()/2, l_oRect.height()/2));

				data_item *l_oData = *m_oControl + l_iId;
				l_oData->m_iXX = l_oR->pos().x();
				l_oData->m_iYY = l_oR->pos().y();

				m_oItems[l_iId] = l_oR;
			}
			break;
		case cmd_update_item:
			{
				if (i_o[data_orig].toInt() == view_canvas) return;
				int j = i_o[data_id].toInt();
				m_oItems.value(j)->update_data();
				m_oItems.value(j)->update();
			}
			break;
		case cmd_select_item:
			{
				if (i_o[data_orig].toInt() == view_canvas)
				{
					check_selected();
					return;
				}
				int l_iId = i_o[data_id].toInt();
				if (l_iId == NO_ITEM)
				{
					deselect_all(false);
				}
				else if (m_oSelected.size() == 1)
				{
					if (m_oSelected[0]->Id() != l_iId)
					{
						rm_select(m_oSelected[0], false);
						add_select(m_oItems.value(l_iId), false);
					}
				}
				else
				{
					add_select(m_oItems.value(l_iId), false);
				}
				check_selected();
			}
			break;
		case cmd_link:
			{
				canvas_item *l_oR1 = m_oItems.value(i_o[data_id].toInt());
				canvas_item *l_oR2 = m_oItems.value(i_o[data_parent].toInt());
				canvas_link * l_oLink = new canvas_link(this, l_oR2, l_oR1);
				l_oLink->update_pos();
				l_oR1->update();
			}
			break;
		case cmd_unlink:
			{
				canvas_item *l_oR1 = m_oItems.value(i_o[data_id1].toInt());
				canvas_item *l_oR2 = m_oItems.value(i_o[data_id2].toInt());

				foreach (QGraphicsItem *l_oItem, items())
				{
					if (l_oItem->type() == gratype(17))
					{
						canvas_link *l_oLink = (canvas_link*) l_oItem;
						if (
							(l_oLink->m_oFrom == l_oR1 && l_oLink->m_oTo == l_oR2)
							||
							(l_oLink->m_oFrom == l_oR2 && l_oLink->m_oTo == l_oR1)
						)
						{
							l_oLink->hide();
							l_oLink->rm_link();
							delete l_oLink;
							break;
						}
					}
				}
				l_oR1->update();
				l_oR2->update();
			}
			break;
		case cmd_remove_item:
			{
				int l_iId = i_o[data_id].toInt();
				canvas_item *l_oR1 = m_oItems.value(l_iId);

				Q_ASSERT(l_oR1!=NULL);

				m_oSelected.removeAll(l_oR1);
				m_oItems.remove(l_iId);

				//l_oR1->hide();
				scene()->removeItem(l_oR1);
				delete l_oR1;
			}
			break;
		case cmd_sort_item:
			{
				show_sort(m_oSelected[0]->Id(), m_iMode == sort_mode);
			}
			break;
		case cmd_save_data:
			{
				QList<canvas_item*> l_oList = m_oItems.values();
				foreach (canvas_item *l_oItem, l_oList)
				{
					data_item *l_oData = *m_oControl + l_oItem->Id();
					l_oData->m_iXX = l_oItem->pos().x();
					l_oData->m_iYY = l_oItem->pos().y();
					l_oData->m_iWW = l_oItem->boundingRect().width();
					l_oData->m_iHH = l_oItem->boundingRect().height();
				}
			}
			break;
		case cmd_update_flags:
			{
				semantik_win *l_o = (semantik_win*) m_oSemantikWindow;
				l_o->m_oFlagsToolBar->clear();

				while (l_o->m_oFlagGroup->actions().size() > m_oControl->m_oFlagSchemes.size())
				{
					QAction* l_oA = l_o->m_oFlagGroup->actions().takeFirst();
					delete l_oA;
				}

				while (l_o->m_oFlagGroup->actions().size() < m_oControl->m_oFlagSchemes.size())
				{
					new QAction(QIcon(), trUtf8("Flag"), l_o->m_oFlagGroup);
				}

				for (int i=0; i<m_oControl->m_oFlagSchemes.size(); ++i)
				{
					flag_scheme* l_oScheme = m_oControl->m_oFlagSchemes[i];
					QAction *l_oAction = l_o->m_oFlagGroup->actions()[i];

					l_oAction->setText(l_oScheme->m_sName);
					l_oAction->setIcon(l_oScheme->_icon());
				}

                                foreach(QAction* l_oAct, l_o->m_oFlagGroup->actions())
                                {
                                        l_o->m_oFlagsToolBar->addAction(l_oAct);
					l_oAct->setCheckable(true);
                                }
			}
			break;
		case cmd_update_colors:
			{
				semantik_win *l_o = (semantik_win*) m_oSemantikWindow;
				l_o->m_oColorsToolBar->clear();

				while (l_o->m_oColorGroup->actions().size() > m_oControl->m_oColorSchemes.size()+1)
				{
					QAction* l_oA = l_o->m_oColorGroup->actions().takeFirst();
					l_o->m_oColorsToolBar->removeAction(l_oA);
					m_oColorMenu->removeAction(l_oA);
					delete l_oA;
				}

				while (l_o->m_oColorGroup->actions().size() < m_oControl->m_oColorSchemes.size()+1)
				{
					new QAction(QIcon(), trUtf8("Color"), l_o->m_oColorGroup);
				}

				l_o->m_oColorGroup->removeAction(l_o->m_oCustomColorAct);
				l_o->m_oColorGroup->addAction(l_o->m_oCustomColorAct);
				m_oColorMenu->removeAction(l_o->m_oCustomColorAct);
				m_oColorMenu->addAction(l_o->m_oCustomColorAct);

				for (int i=0; i<m_oControl->m_oColorSchemes.size(); ++i)
				{
					color_scheme l_oScheme = m_oControl->m_oColorSchemes[i];
					QAction *l_oAction = l_o->m_oColorGroup->actions()[i];

					QPixmap l_oPix(22, 22);
					QPainter l_oP(&l_oPix);

					l_oAction->setText(l_oScheme.m_sName);

					l_oPix.fill(l_oScheme.m_oInnerColor);
					//TODO pen ?
					l_oP.drawRect(0, 0, 21, 21);
					l_oAction->setIcon(QIcon(l_oPix));
				}

				// la première action est pour la couleur de la racine
				for (int i=1; i<l_o->m_oColorGroup->actions().size(); ++i)
				{
					QAction *l_oAct = l_o->m_oColorGroup->actions()[i];
					l_o->m_oColorsToolBar->addAction(l_oAct);
					m_oColorMenu->addAction(l_oAct);
				}
			}
			break;
		case cmd_open_map:
			{
				check_canvas_size();
				fit_zoom();
			}
			break;
		default:
			break;
	}
}

void canvas_view::change_colors(QAction* i_oAct)
{
	static QColor l_oColor;

	int l_iIdx = -1;
	semantik_win *l_o = (semantik_win*) m_oSemantikWindow;
	for (int i=1; i<l_o->m_oColorGroup->actions().size(); ++i)
	{
		// attention hat trick, il y a une assignation
		if (l_o->m_oColorGroup->actions()[i]==i_oAct && (l_iIdx = i))
		{
			if (i==l_o->m_oColorGroup->actions().size()-1)
			{
				l_oColor = QColorDialog::getColor(l_oColor, this);
				if (!l_oColor.isValid()) return;
			}
			break;
		}
	}

	for (int i=0; i<m_oSelected.size(); ++i)
	{
		canvas_item *l_oItem = m_oSelected[i];
		int l_oId = l_oItem->Id();
		data_item *l_oData = *m_oControl + l_oId;
		l_oData->m_oCustom.m_oInnerColor = l_oColor;
		l_oData->m_oCustom.m_oBorderColor = l_oColor.dark();
		l_oData->m_iXX = l_oItem->pos().x();
		l_oData->m_iYY = l_oItem->pos().y();
		l_oData->m_iColor = l_iIdx;
		m_oControl->update_item(l_oId);
	}
}

void canvas_view::change_flags(QAction* i_oAction)
{
	int l_iIndex = 0;
	semantik_win *l_o = (semantik_win*) m_oSemantikWindow;
	for (int i=0; i<l_o->m_oFlagGroup->actions().size(); ++i)
	{
		QAction *l_oAct = l_o->m_oFlagGroup->actions()[i];
		if (l_oAct == i_oAction) l_iIndex = i;
	}
	QString l_sName = m_oControl->m_oFlagSchemes[l_iIndex]->m_sId;
	canvas_item *l_oItem = m_oSelected[0];
	data_item *l_oData = *m_oControl + m_oSelected[0]->Id();

	if (i_oAction->isChecked()) l_oData->m_oFlags.push_back(l_sName);
	else l_oData->m_oFlags.removeAll(l_sName);
	l_oItem->update_flags();
}

void canvas_view::check_selected()
{
	semantik_win *l_o = (semantik_win*) m_oSemantikWindow;
	bool l_bItemsAreSelected = (m_oSelected.size()==1);
	data_item *l_oData = NULL;
	if (l_bItemsAreSelected)
	{
		l_oData = *m_oControl + m_oSelected[0]->Id();
	}

	//foreach(QAction* l_oAct, l_o->m_oFlagGroup->actions())
	for (int i=0; i<l_o->m_oFlagGroup->actions().size(); ++i)
	{
		QAction *l_oAct = l_o->m_oFlagGroup->actions()[i];

		if (!l_bItemsAreSelected) l_oAct->setChecked(false);
		else
		{
			QString l_sName = m_oControl->m_oFlagSchemes[i]->m_sId;
			l_oAct->setChecked(l_oData->m_oFlags.contains(l_sName));
		}
		l_oAct->setEnabled(l_bItemsAreSelected);
	}
	l_bItemsAreSelected = (m_oSelected.size()>=1);
	foreach(QAction* l_oAct, l_o->m_oColorGroup->actions())
	{
		l_oAct->setEnabled(l_bItemsAreSelected);
	}
}

void canvas_view::deselect_all(bool i_oSignal)
{
	if (m_oSelected.size() == 1)
	{
		QFocusEvent l_oEv = QFocusEvent(QEvent::FocusOut);
		m_oSelected[0]->focus_out(&l_oEv);
	}

	foreach (canvas_item *l_oItem, m_oSelected)
	{
		l_oItem->set_selected(false);
		l_oItem->setZValue(99);
		if (m_iMode == sort_mode || m_iLastMode == sort_mode)
		{
			show_sort(l_oItem->Id(), false);
			emit sig_message("", -1);
		}
		l_oItem->update();
	}
	m_oSelected.clear();

	notify_select(i_oSignal);
}

void canvas_view::add_select(canvas_item* i_oItem, bool i_oSignal)
{
	Q_ASSERT(i_oItem != NULL);

	if (m_oSelected.size() == 1)
        {
		QFocusEvent l_oEv = QFocusEvent(QEvent::FocusOut);
		m_oSelected[0]->focus_out(&l_oEv);
        }

	m_iSortCursor = 0;
	m_oSelected.push_back(i_oItem);
	i_oItem->set_selected(true);
	i_oItem->setZValue(100);
	if (m_iMode == sort_mode)
	{
		show_sort(i_oItem->Id(), true);
		emit sig_message(trUtf8("Click to set Item %1").arg(QString::number(m_iSortCursor+1)), -1);
	}
	i_oItem->update();

	notify_select(i_oSignal);
}

void canvas_view::rm_select(canvas_item* i_oItem, bool i_oSignal)
{
	Q_ASSERT(i_oItem != NULL);
	i_oItem->set_selected(false);
	i_oItem->setZValue(99);
	if (m_iMode == sort_mode) show_sort(i_oItem->Id(), false);
	i_oItem->update();
	m_oSelected.removeAll(i_oItem);

	notify_select(i_oSignal);
}

void canvas_view::notify_select(bool i_oSignal)
{
	enable_actions();

	if (!i_oSignal) return;
	if (m_oSelected.size()==1)
		m_oControl->select_item(m_oSelected[0]->Id(), view_canvas);
	else
		m_oControl->select_item(NO_ITEM, view_canvas);
}

void canvas_view::enable_actions()
{
	foreach (QAction* l_o, actions())
	{
		l_o->setEnabled(true); // TODO we could disable the move actions one by one
	}
	enable_menu_actions();
}

void canvas_view::enable_menu_actions()
{
	m_oAddItemAction->setEnabled(m_oSelected.size()<=1);
	m_oDeleteAction->setEnabled(m_oSelected.size()>0);
	m_oEditAction->setEnabled(m_oSelected.size()==1);
	m_oColorMenu->setEnabled(m_oSelected.size()>=1);
	m_oDataMenu->setEnabled(m_oSelected.size()==1);
	foreach (QAction* l_o, m_oDataMenu->actions())
        {
                l_o->setEnabled(m_oSelected.size()==1);
        }
}

void canvas_view::mousePressEvent(QMouseEvent *i_oEv)
{
	if (i_oEv->button() == Qt::RightButton)
	{
		// select the item under the cursor if available and show the popup menu
		m_oLastPoint = mapToScene(i_oEv->pos());
		QGraphicsItem *l_oItem = scene()->itemAt(mapToScene(i_oEv->pos()));
		if (l_oItem && l_oItem->type() == gratype(7))
		{
			if (!m_oSelected.contains((canvas_item*) l_oItem))
			{
				deselect_all();
				add_select((canvas_item*) l_oItem);
			}
		}
		else
		{
			deselect_all();
		}
		m_oMenu->popup(i_oEv->globalPos());
		return;
	}

	if (i_oEv->button() == Qt::MidButton)
	{
		m_iLastMode = m_iMode;
		m_iMode = scroll_mode;
	}

	canvas_item *l_oRect = NULL;
	switch (m_iMode)
	{
		case select_mode:
			{
				m_oLastPressPoint = i_oEv->pos();
				m_oLastMovePoint = mapToScene(m_oLastPressPoint);


				m_bPressed = (i_oEv->button() == Qt::LeftButton);

				QGraphicsItem *l_oItem = scene()->itemAt(mapToScene(i_oEv->pos()));
				if (m_bPressed)
				{
					if (l_oItem && l_oItem->type() == gratype(7))
					{
						l_oRect = (canvas_item*) l_oItem;
					}
					else
					{
						l_oItem = NULL;
					}

					if (l_oItem)
					{
						switch (i_oEv->modifiers())
						{
		case Qt::ShiftModifier:
			if (m_oSelected.contains(l_oRect))
				rm_select(l_oRect);
			else
				add_select(l_oRect);
			break;
		default:
			if (!m_oSelected.contains(l_oRect))
			{
				if (m_oSelected.size() == 1 && !m_oControl->parent_of(l_oRect->Id()))
				{
					if (QApplication::keyboardModifiers() & Qt::ControlModifier)
						m_oControl->link_items(m_oSelected[0]->Id(), l_oRect->Id());
				}
				deselect_all();
				add_select(l_oRect);
			}
						}
					}
					else
					{
						deselect_all();
					}
				}

			}
			break;
		case link_mode:
			{
				m_oLastPressPoint = i_oEv->pos();
				m_bPressed = (i_oEv->button() == Qt::LeftButton);

				deselect_all();
				QGraphicsItem *l_oItem = scene()->itemAt(mapToScene(i_oEv->pos()));
				if (l_oItem && l_oItem->type() == gratype(7))
				{
					l_oRect = (canvas_item*) l_oItem;
					add_select(l_oRect);
				}
			}
			break;
		case sort_mode:
			{
				QGraphicsItem *l_oItem = scene()->itemAt(mapToScene(i_oEv->pos()));
				if (l_oItem && l_oItem->type() == gratype(7))
				{
					l_oRect = (canvas_item*) l_oItem;
					deselect_all();
					add_select(l_oRect);
				}
				else if (l_oItem && l_oItem->type() == gratype(19))
				{
					qDebug()<<"sort called"<<endl;

					canvas_sort * l_oSort = (canvas_sort*) l_oItem;
					int l_iId = l_oSort->m_oFrom->Id();
					l_oRect = l_oSort->m_oFrom;

					qDebug()<<"sort_children("<<m_oSelected[0]->Id()<<" "<<l_iId<<" "<<m_iSortCursor;

					m_oControl->sort_children(m_oSelected[0]->Id(), l_iId, m_iSortCursor);
					show_sort(m_oSelected[0]->Id(), true);

					m_iSortCursor++;
					if (m_iSortCursor >= m_oControl->num_children(m_oSelected[0]->Id()))
						m_iSortCursor=0;

					emit sig_message(trUtf8("Click to set Item %1").arg(QString::number(m_iSortCursor+1)), -1);
					//qDebug();
				}
				else
				{
					deselect_all();
				}
			}
			break;
		case scroll_mode:
			viewport()->setCursor(Qt::ClosedHandCursor);
			m_oLastPressPoint = i_oEv->pos();
			m_bPressed = (i_oEv->button() == Qt::LeftButton || i_oEv->button() == Qt::MidButton);

			if (m_bPressed)
			{
				deselect_all();
                                QGraphicsItem *l_oItem = scene()->itemAt(mapToScene(i_oEv->pos()));
                                if (l_oItem && l_oItem->type() == gratype(7))
                                {
                                        l_oRect = (canvas_item*) l_oItem;
                                        add_select(l_oRect);
				}
			}

			break;
		default:
			break;
	}
}

void canvas_view::mouseMoveEvent(QMouseEvent *i_oEv)
{
	check_canvas_size();
	switch (m_iMode)
	{
		case select_mode:
			if (m_bPressed && m_oSelected.size() == 0)
			{
				bool c1 = ((m_oLastPressPoint - i_oEv->pos()).manhattanLength() >= QApplication::startDragDistance());
				if (m_oRubbery->isVisible() || c1)
				{
					QRect l_oSel = QRect(m_oLastPressPoint, i_oEv->pos()).normalized();
					m_oRubbery->setGeometry(l_oSel);
				}

				if (c1) m_oRubbery->show();
			}
			else if (m_bPressed)
			{
				// ugly, but it works

				QRectF l_oRect;
				l_oRect.setTopLeft(m_oLastMovePoint);
				m_oLastMovePoint = mapToScene(i_oEv->pos());
				l_oRect.setBottomRight(m_oLastMovePoint);
				m_oControl->set_dirty();
				foreach (canvas_item *l_oItem, m_oSelected)
				{
					l_oItem->moveBy(l_oRect.width(), l_oRect.height());
				}
				// FIXME
				foreach (canvas_item *l_oItem, m_oSelected)
				{
					l_oItem->update_links();
				}
			}
			break;
		case link_mode:
			if (m_bPressed)
			{
				bool c1 = ((m_oLastPressPoint - i_oEv->pos()).manhattanLength() >= QApplication::startDragDistance());
				if (m_oRubberLine->isVisible() || c1)
				{
					QRect l_oSel = QRect(m_oLastPressPoint, i_oEv->pos());
					m_oRubberLine->setGeometry(l_oSel);
				}

				if (c1)
				{
					m_oRubberLine->show();
				}
			}
			break;
		case sort_mode:
			break;
		case scroll_mode:
			if (m_bPressed)
			{
				QScrollBar *hBar = horizontalScrollBar();
				QScrollBar *vBar = verticalScrollBar();

				QPoint delta = i_oEv->pos() - m_oLastPressPoint;

				hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
				vBar->setValue(vBar->value() - delta.y());

				m_oLastPressPoint = i_oEv->pos();

				//viewport()->setCursor(Qt::ClosedHandCursor);
			}
			else
			{
				//viewport()->setCursor(Qt::OpenHandCursor);
			}
		default:
			break;
	}
}

void canvas_view::mouseReleaseEvent(QMouseEvent *i_oEv)
{
	if (i_oEv->button() == Qt::RightButton) return;

	if (m_iLastMode != no_mode)
	{
		m_iMode = m_iLastMode;
		m_iLastMode = no_mode;
		viewport()->setCursor(m_iMode==link_mode?Qt::CrossCursor:Qt::ArrowCursor);
	}

	switch (m_iMode)
	{
		case select_mode:
			{
				m_bPressed = false;
				if (m_oRubbery->isVisible())
				{
					m_oRubbery->hide();
					QRect l_oRect = m_oRubbery->geometry();
					QList<QGraphicsItem *> l_oSel = scene()->items(QRectF(mapToScene(l_oRect.topLeft()), mapToScene(l_oRect.bottomRight())));

					//ITA

					foreach (QGraphicsItem *l_oItem, l_oSel)
					{
						if (l_oItem->type() == gratype(7))
						{
							add_select((canvas_item*) l_oItem);
						}
					}
				}
			}
			break;
		case link_mode:
			{
				m_bPressed = false;

				canvas_item *l_oR1 = NULL;
				canvas_item *l_oR2 = NULL;

				foreach (QGraphicsItem *l_oI1, scene()->items(mapToScene(m_oLastPressPoint)))
				{
					if (l_oI1->type() == gratype(7))
					{
						l_oR1 = (canvas_item*) l_oI1;
						break;
					}
				}

				foreach (QGraphicsItem *l_oI1, scene()->items(mapToScene(i_oEv->pos())))
				{
					if (l_oI1->type() == gratype(7))
					{
						l_oR2 = (canvas_item*) l_oI1;
						break;
					}
				}

				if (l_oR1 && l_oR2 && l_oR1 != l_oR2)
				{
					m_oControl->link_items(l_oR1->Id(), l_oR2->Id());
					deselect_all();
				}
				m_oRubberLine->hide();
			}
			break;
		case sort_mode:
			break;
		case scroll_mode:
			viewport()->setCursor(Qt::OpenHandCursor);
			m_bPressed = false;
			break;
		default:
			break;
	}
}

void canvas_view::mouseDoubleClickEvent(QMouseEvent* i_oEv)
{
	if (i_oEv->button() != Qt::LeftButton) return;
	m_oLastPoint = mapToScene(i_oEv->pos());
	QGraphicsItem *l_oItem = itemAt(i_oEv->pos());
	int l_iAdded = NO_ITEM;
	if (m_iMode == select_mode || m_iMode == link_mode)
	{
		if (l_oItem)
		{
			if (l_oItem->type() == gratype(7))
			{
				deselect_all();

				canvas_item *l_oR = (canvas_item*) l_oItem;
				l_oR->setSelected(false);
				l_iAdded = m_oControl->add_item(l_oR->Id(), NO_ITEM, true);
				if (m_oSelected.size() == 1) m_oSelected[0]->focus_in();
				check_canvas_size();
			}
			else if (l_oItem->type() == gratype(17))
			{
				canvas_link *l_oLink = (canvas_link*) l_oItem;
				m_oControl->unlink_items(l_oLink->m_oTo->Id(), l_oLink->m_oFrom->Id());
			}
		}
		else if (i_oEv->modifiers() != Qt::ControlModifier)
		{
			l_iAdded = m_oControl->add_item();
			if (m_oSelected.size() == 1) m_oSelected[0]->focus_in();
		}
	}
}

qreal _min(qreal a, qreal b) { if (a<b) return a; return b; }
qreal _max(qreal a, qreal b) { if (a<b) return b; return a; }

void canvas_view::check_canvas_size()
{
	// using viewport()->rect() was a bad idea
	QRect l_oRect = viewport()->rect();
	if (m_oItems.size() < 1) // no rectangle
	{
		scene()->setSceneRect(QRectF(mapToScene(l_oRect.topLeft()), mapToScene(l_oRect.bottomRight())));
		return;
	}

	QRectF l_oR2;

	qreal x, y, z, t;
	canvas_item *l_o = m_oItems.values()[0];

	x = z = l_o->x() + l_o->rect().width()/2;
	y = t = l_o->y() + l_o->rect().height()/2;

	QList<canvas_item*> l_oList = m_oItems.values();
	foreach (canvas_item *l_oItem, l_oList)
	{
		if (l_oItem->x() < x) x = l_oItem->x();
		if (l_oItem->y() < y) y = l_oItem->y();
		if (l_oItem->x2() > z) z = l_oItem->x2();
		if (l_oItem->y2() > t) t = l_oItem->y2();
	}
	x -=100; y -= 100; z += 100, t += 100;

	l_oR2 = QRectF(QPointF(x, y), QPointF(z, t));
        l_oR2 = l_oR2.united(QRectF(mapToScene(l_oRect.topLeft()), mapToScene(l_oRect.bottomRight())));
	if (l_oR2 == sceneRect()) return;
	scene()->setSceneRect(l_oR2);
}

void canvas_view::fit_zoom()
{
	//QRectF l_o = scene()->sceneRect();
	//fitInView(QRectF(l_o.topLeft()+QPointF(100, 100), l_o.size()+QSizeF(200, -200)), Qt::KeepAspectRatio);
#if 0
	QRectF l_o;
	foreach (QGraphicsItem *it, items())
	{
		l_o |= it->boundingRect();
	}
#endif

	fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}

void canvas_view::focusInEvent(QFocusEvent *i_oEv)
{
	enable_actions();
}

void canvas_view::slot_change_data()
{
	m_oControl->change_data(m_oSelected[0]->Id(), ((QAction*) QObject::sender())->data().toInt());
}

rubber_line::rubber_line(QRubberBand::Shape i, QWidget* j) : QRubberBand(i, j)
{
}

void rubber_line::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
    //painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(Qt::red));
	if (_direction > 0)
	{
		painter.drawLine(QPoint(0, 0), QPoint(size().width(), size().height()));
	}
	else
	{
		painter.drawLine(QPoint(size().width(), 0), QPoint(0, size().height()));
	}
}

void rubber_line::setGeometry(const QRect& i_o)
{
	_direction = i_o.width() * i_o.height();
	QRubberBand::setGeometry(i_o.normalized());
}

 %: include  	"canvas_view.moc" 
