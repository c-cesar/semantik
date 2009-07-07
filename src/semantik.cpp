// Thomas Nagy 2007-2009 GPLV3

#include <QtGui>
#include <QTextBrowser>
#include <QProgressDialog>
#include <KStatusBar>
#include <KFileDialog>
#include <KConfigGroup>
#include <KMenuBar>
#include <KApplication>
#include <KStandardAction>
#include <KRecentFilesAction>
#include <KActionCollection>
#include<KToolBar>
#include <KAction>
#include <KMenu>
#include <KMessageBox>
#include <ktip.h>

#include "box_view.h"
#include  "sembind.h"
#include "semantik.h"
#include  "canvas_view.h"
#include   "data_control.h"
#include    "image_view.h"
#include "config_dialog.h"
#include   "text_view.h"
#include    "table_view.h"
#include       "vars_view.h"
#include      "pre_view.h"
#include     "linear_container.h"
#include    "template_entry.h"
#include   "generator_dialog.h"
#include  "data_view.h"
#include    "doc_dialog.h"
#include    "aux.h" 

void semantik_win::slot_generate()
{
	if (m_oControl->choose_root() <= 0)
	{
		KMessageBox::sorry(this, trUtf8("The map is empty, Semantik\ncannot create documents from it"), trUtf8("Empty map"));
		return;
	}

	QDir l_oDir(TEMPLATE_DIR);
	if (!l_oDir.exists() || !l_oDir.isReadable())
	{
		slot_message(trUtf8("Template folder %1 is not readable").arg(TEMPLATE_DIR), 5000);
		return;
	}
	QFileInfoList l_o = l_oDir.entryInfoList(QStringList()<<"*.py", QDir::Files, QDir::Name);
	//qDebug()<<l_o.size();

	QList<template_entry> l_oLst;
	foreach (QFileInfo l_oInfo, l_o)
	{
		template_entry l_oTemplate;
		int l_iCnt = 0;
		if (l_oInfo.isReadable())
		{
			QFile l_oFile(l_oInfo.absoluteFilePath());
			if (!l_oFile.open(QIODevice::ReadOnly))
			{
				//qDebug()<<"could not open the file"<<l_oInfo.absoluteFilePath();
				continue;
			}

			if (l_oFile.fileName().contains(notr("semantik.sem.py")))
			{
				continue;
			}

			l_oTemplate.m_sFile = l_oInfo.absoluteFilePath();

			QTextStream l_oS(&l_oFile);
			while (!l_oS.atEnd())
			{
				QString l_sLine = l_oS.readLine(1000);
				if (l_sLine.length() < 2)
				{
					continue;
				}
				else if (l_sLine.startsWith("#"))
				{
					if (l_sLine.startsWith(notr("#sem:name:")))
					{
						l_oTemplate.m_sName = l_sLine.mid(11);
						l_iCnt++;
					}
					else if (l_sLine.startsWith(notr("#sem:tip:")))
					{
						l_oTemplate.m_sTip = l_sLine.mid(10);
						l_iCnt++;
					}
				}
				else
				{
					break;
				}

			}
			l_oFile.close();

			if (l_iCnt<2) continue;
			l_oLst.push_front(l_oTemplate);
			//qDebug()<<l_oTemplate.m_sFile;
			//qDebug()<<l_oTemplate.m_sName;
			//qDebug()<<l_oTemplate.m_sTip;
		}
	}

	generator_dialog l_oGen(this, l_oLst);
	l_oGen.m_oName->setText(m_oControl->m_sOutProject);
	l_oGen.m_oLocation->setText(m_oControl->m_sOutDir);
	if (l_oGen.m_oLocation->text().length()<2) l_oGen.m_oLocation->setText(bind_node::get_var(notr("outdir")));

	l_oGen.activate_from_name(m_oControl->m_sOutTemplate);
	if (l_oGen.exec() == QDialog::Accepted)
	{
		m_oControl->m_sOutDir = l_oGen.m_oLocation->text();
		m_oControl->m_sOutProject = l_oGen.m_oName->text();
		m_oControl->m_sOutTemplate = l_oGen.m_sShortName;
		m_oControl->generate_docs(l_oGen.m_sCurrent, l_oGen.m_oName->text(), l_oGen.m_oLocation->text());
	}
}

semantik_win::semantik_win(QWidget *i_oParent) : KXmlGuiWindow(i_oParent)
{
	setObjectName("semantik_win");

	//setWindowIcon(QIcon(notr(":images/semantik-32.png")));

	m_oControl = new data_control(this);
	connect(m_oControl, SIGNAL(sig_message(const QString&, int)), this, SLOT(slot_message(const QString&, int)));

	m_oCanvas = new canvas_view(this, m_oControl);
	setCentralWidget(m_oCanvas);
	connect(m_oCanvas, SIGNAL(sig_message(const QString&, int)), this, SLOT(slot_message(const QString&, int)));

	//BEGIN setup_actions();
	KStandardAction::quit(this, SLOT(close()), actionCollection());
	KStandardAction::save(this, SLOT(slot_save()), actionCollection());
	KStandardAction::saveAs(this, SLOT(slot_save_as()), actionCollection());
	KStandardAction::open(this, SLOT(slot_open()), actionCollection());
	KStandardAction::print(this, SLOT(slot_print()), actionCollection());
	KStandardAction::tipOfDay(this, SLOT(slot_tip_of_day()), actionCollection());

	m_oRecentFilesAct = KStandardAction::openRecent(this, SLOT(slot_recent(const KUrl&)), actionCollection());

	m_oReorgAct = new KAction(trUtf8("&Reorganize the map"), this);
	actionCollection()->addAction(notr("reorganize_map"), m_oReorgAct);
	m_oReorgAct->setShortcut(trUtf8("Ctrl+R"));
	connect(m_oReorgAct, SIGNAL(triggered(bool)), m_oControl, SLOT(do_reorganize()));

	KStandardAction::preferences(this, SLOT(slot_properties()), actionCollection());

	m_oGenerateAct = new KAction(trUtf8("&Generate..."), this);
	actionCollection()->addAction(notr("generate_doc"), m_oGenerateAct);
	m_oGenerateAct->setIcon(KIcon(notr("run-build-file")));
	m_oGenerateAct->setShortcut(trUtf8("Ctrl+G"));
	connect(m_oGenerateAct, SIGNAL(triggered(bool)), SLOT(slot_generate()));

	m_oGroup = new QActionGroup(this);

	m_oSelectAct = new KAction(trUtf8("&Select"), this);
	actionCollection()->addAction(notr("tool_select"), m_oSelectAct);
	connect(m_oSelectAct, SIGNAL(triggered(bool)), SLOT(slot_tool_select()));
	m_oSelectAct->setIcon(KIcon(notr("semantik_point")));
	m_oSelectAct->setCheckable(true);

	m_oLinkAct = new KAction(trUtf8("&Link"), this);
	actionCollection()->addAction(notr("tool_link"), m_oLinkAct);
	connect(m_oLinkAct, SIGNAL(triggered(bool)), SLOT(slot_tool_link()));
	m_oLinkAct->setIcon(KIcon(notr("semantik_link")));
	m_oLinkAct->setCheckable(true);

	m_oSortAct = new KAction(trUtf8("&Sort"), this);
	actionCollection()->addAction(notr("tool_sort"), m_oSortAct);
	connect(m_oSortAct, SIGNAL(triggered(bool)), SLOT(slot_tool_sort()));
	m_oSortAct->setIcon(KIcon(notr("semantik_sort")));
	m_oSortAct->setCheckable(true);

	m_oScrollAct = new KAction(trUtf8("&Scroll"), this);
	actionCollection()->addAction(notr("tool_scroll"), m_oScrollAct);
	connect(m_oScrollAct, SIGNAL(triggered(bool)), SLOT(slot_tool_scroll()));
	m_oScrollAct->setIcon(KIcon(notr("semantik_move")));
	m_oScrollAct->setCheckable(true);

	m_oGroup->addAction(m_oSelectAct);
	m_oGroup->addAction(m_oLinkAct);
	m_oGroup->addAction(m_oSortAct);
	m_oGroup->addAction(m_oScrollAct);

	m_oSelectAct->setChecked(true);

	flag_scheme l_oScheme(this, notr("crsc-app-colors"), "");

	m_oColorGroup = new QActionGroup(this);
	m_oCustomColorAct = new QAction(l_oScheme._icon(), trUtf8("CustomColor"), m_oColorGroup);
	m_oColorGroup->setExclusive(true);
	connect(m_oColorGroup, SIGNAL(triggered(QAction*)), m_oCanvas, SLOT(change_colors(QAction*)));

	m_oFlagGroup = new QActionGroup(this);
	m_oFlagGroup->setExclusive(false);
	connect(m_oFlagGroup, SIGNAL(triggered(QAction*)), m_oCanvas, SLOT(change_flags(QAction*)));

	slot_tool_select();

	//m_oCanvasFitZoom = new KAction(trUtf8("Fit zoom"), this);
	//actionCollection()->addAction(notr("zoom_map"), m_oCanvasFitZoom);
	//m_oCanvasFitZoom->setIcon(KIcon(notr("zoom-best-fit")));
	//connect(m_oCanvasFitZoom, SIGNAL(triggered(bool)), m_oCanvas, SLOT(fit_zoom()));
	m_oCanvasFitZoom = KStandardAction::fitToPage(m_oCanvas, SLOT(fit_zoom()), actionCollection());
	m_oCanvasFitZoom->setIcon(KIcon(notr("zoom-best-fit")));
	m_oCanvasFitZoom->setShortcut(trUtf8("Ctrl+H"));

	KStandardAction::zoomIn(m_oCanvas, SLOT(zoom_in()), actionCollection());
	KStandardAction::zoomOut(m_oCanvas, SLOT(zoom_out()), actionCollection());

	//END setupActions()
	setStandardToolBarMenuEnabled(true);


	QDockWidget *l_oDockData = new QDockWidget(trUtf8("Data"), this);
	l_oDockData->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	m_oDataView = new data_view(l_oDockData, m_oControl);
	l_oDockData->setWidget(m_oDataView);
	addDockWidget(Qt::BottomDockWidgetArea, l_oDockData);
	actionCollection()->addAction(notr("show_dock_data"), l_oDockData->toggleViewAction());
	l_oDockData->setObjectName(notr("DataDock"));

	m_oTextView = new text_view(m_oDataView, m_oControl);
	m_oDataView->addWidget(m_oTextView);

	m_oImageView = new image_view(m_oDataView, m_oControl);
	m_oDataView->addWidget(m_oImageView);

	m_oTableView = new table_view(m_oDataView, m_oControl);
	m_oDataView->addWidget(m_oTableView);

	m_oDiagramView = new box_view(m_oDataView, m_oControl);
	m_oDataView->addWidget(m_oDiagramView);


	QDockWidget *l_oDockVars = new QDockWidget(trUtf8("Variables"), this);
	l_oDockVars->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	m_oVarsView = new vars_view(l_oDockVars, m_oControl);
	l_oDockVars->setWidget(m_oVarsView);
	addDockWidget(Qt::BottomDockWidgetArea, l_oDockVars);
	l_oDockVars->setObjectName(notr("VarsDock"));
	actionCollection()->addAction(notr("show_dock_vars"), l_oDockVars->toggleViewAction());
	m_oVarsView->init_completer();

	QDockWidget *l_oDockPreview = new QDockWidget(trUtf8("Preview"), this);
	l_oDockPreview->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
	m_oPreView = new pre_view(l_oDockPreview, m_oControl);
	l_oDockPreview->setWidget(m_oPreView);
	addDockWidget(Qt::RightDockWidgetArea, l_oDockPreview);
	actionCollection()->addAction(notr("show_dock_preview"), l_oDockPreview->toggleViewAction());
	l_oDockPreview->setObjectName(notr("PreviewDock"));


	tabifyDockWidget(l_oDockData, l_oDockVars);


	QDockWidget *l_oDock = new QDockWidget(trUtf8("Linear view"), this);
	m_oTree = new linear_container(l_oDock, m_oControl);
	l_oDock->setWidget(m_oTree);
	addDockWidget(Qt::LeftDockWidgetArea, l_oDock);
	actionCollection()->addAction(notr("show_dock_linear"), l_oDock->toggleViewAction());
	l_oDock->setObjectName(notr("LinearDock"));


	setXMLFile(notr("semantikui.rc"));
	setupGUI();

	set_current_file(KUrl());

	m_oColorsToolBar = toolBar(notr("colorsToolBar"));
	m_oFlagsToolBar = toolBar(notr("flagsToolBar"));

	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oCanvas, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oTableView, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oImageView, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oTextView, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oDataView, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oTree, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oVarsView, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oPreView, SLOT(synchro_doc(const hash_params&)));
	connect(m_oControl, SIGNAL(synchro(const hash_params&)), m_oDiagramView, SLOT(synchro_doc(const hash_params&)));

	connect(m_oImageView, SIGNAL(sig_message(const QString&, int)), this, SLOT(slot_message(const QString&, int)));

	//qDebug()<<"instance is "<<l_oInst<<endl;

	m_oControl->init_colors();
	m_oControl->init_flags();

	m_oControl->select_item(NO_ITEM);

	read_config();
	statusBar()->showMessage(trUtf8("Welcome to Semantik"), 2000);
	setAutoSaveSettings();


	KTipDialog::showTip(this, notr("semantik/tips"));
}

void semantik_win::read_config()
{
	KConfigGroup l_oConfig(KGlobal::config(), notr("General Options"));
	m_oRecentFilesAct->loadEntries(KGlobal::config()->group(notr("Recent Files")));
	move(l_oConfig.readEntry(notr("winpos"), QPoint(0, 0)));
	m_oCanvas->setBackgroundBrush(QColor(l_oConfig.readEntry(notr("bgcolor"), notr("#FFFDE8"))));
	m_oControl->m_sOutDir = l_oConfig.readEntry(notr("outdir"), notr("/tmp/"));
	bind_node::set_var(notr("outdir"), m_oControl->m_sOutDir);

	//QString lang_code(config.readEntry("currentLanguageCode", QVariant(QString())).toString());
	//if (lang_code.isEmpty()) lang_code = "en_US";  // null-string are saved as empty-strings
	//setCurrentLanguage(lang_code);
}

void semantik_win::write_config()
{
	KConfigGroup l_oConfig(KGlobal::config(), notr("General Options"));
	m_oRecentFilesAct->saveEntries(KGlobal::config()->group(notr("Recent Files")));
	l_oConfig.writeEntry(notr("winpos"), pos());
	l_oConfig.writeEntry(notr("outdir"), bind_node::get_var(notr("outdir")));
	//config.writeEntry("currentLanguageCode", currentLanguageCode);
	l_oConfig.sync();
}

semantik_win::~semantik_win()
{

}

void semantik_win::slot_print()
{
        QPrinter *l_oP = new QPrinter;
        QPrintDialog l_oD(l_oP, m_oCanvas);
        if (l_oD.exec() == QDialog::Accepted)
        {
                QPainter l_oPa(l_oP);
                m_oCanvas->render(&l_oPa);
        }
        statusBar()->showMessage(trUtf8("Ready"), 2000);
}

bool semantik_win::slot_save_as()
{
	KUrl l_o = KFileDialog::getSaveUrl(KUrl(notr("kfiledialog:///document")),
		trUtf8("*.sem|Semantik file (*.sem)"), this,
		trUtf8("Choose a file name"));

	if (l_o.path().isEmpty()) return false;
	if (!l_o.path().endsWith(notr(".sem")))
	{
		l_o = KUrl(l_o.path()+notr(".sem"));
	}

	if (m_oControl->save_file(l_o.path()))
	{
		statusBar()->showMessage(trUtf8("Saved '%1'").arg(l_o.path()), 2000);
		set_current_file(l_o);
		return true;
	}
	return false;
}

bool semantik_win::slot_save()
{
	if (m_oControl->m_sLastSaved.isEmpty())
	{
		return slot_save_as();
	}
	if (m_oControl->save_file(m_oControl->m_sLastSaved))
	{
		statusBar()->showMessage(trUtf8("Saved '%1'").arg(m_oControl->m_sLastSaved), 2000);
		return true;
	}
	return false;
}

void semantik_win::slot_open()
{
	KUrl l_o = KFileDialog::getOpenUrl(KUrl(notr("kfiledialog:///document")),
		trUtf8("*.sem *.kdi *.mm *.vym|All Supported Files (*.sem *.kdi *.mm *.vym)"),
		this, trUtf8("Choose a file name"));
	if (l_o.isValid() && m_oControl->open_file(l_o.path()))
	{
		set_current_file(l_o);
	}
}

void semantik_win::set_current_file(const KUrl& i_oUrl)
{
	if (i_oUrl.path().isEmpty())
	{
		setWindowTitle(trUtf8("Semantik"));
		return;
	}
	setWindowTitle(trUtf8("%1 - Semantik").arg(i_oUrl.path()));
	m_oRecentFilesAct->addUrl(i_oUrl);
}

void semantik_win::slot_properties()
{
	config_dialog l_oGen(this);

	KConfigGroup l_oSettings(KGlobal::config(), notr("General Options"));
	l_oGen.m_oConnType->setCurrentIndex(l_oSettings.readEntry(notr("conn"), 0));
	l_oGen.m_oReorgType->setCurrentIndex(l_oSettings.readEntry(notr("reorg"), 0));
	l_oGen.m_oAutoSave->setValue(l_oSettings.readEntry(notr("auto"), 5));

	QString l_o = l_oSettings.readEntry(notr("bgcolor"), notr("#FFFDE8"));
	l_oGen.m_oColorWidget->setText(l_o);
	QPalette l_oPalette = l_oGen.m_oColorWidget->palette();
	l_oGen.m_oColor = QVariant(l_o).value<QColor>();
	l_oPalette.setBrush(QPalette::Active, QPalette::Button, l_oGen.m_oColor);
	l_oGen.m_oColorWidget->setPalette(l_oPalette);

	if (l_oGen.exec() == QDialog::Accepted)
	{
		//m_oControl->m_iConnType = l_oGen.m_oConnType->currentIndex();
		//m_oControl->m_iReorgType = l_oGen.m_oReorgType->currentIndex();
		l_oSettings.writeEntry(notr("conn"), m_oControl->m_iConnType = l_oGen.m_oConnType->currentIndex());
		l_oSettings.writeEntry(notr("reorg"), m_oControl->m_iReorgType = l_oGen.m_oReorgType->currentIndex());
		l_oSettings.writeEntry(notr("auto"), m_oControl->m_iTimerValue = l_oGen.m_oAutoSave->value());
		l_oSettings.writeEntry(notr("bgcolor"), l_oGen.m_oColor.name());
		m_oControl->init_timer();

		m_oCanvas->setBackgroundBrush(l_oGen.m_oColor);
	}
}

bool semantik_win::queryClose()
{
	write_config();
	if (!m_oControl->m_bDirty)
	{
		//writeConfig();
		return true;
	}

	QString l_oTitle = m_oControl->m_sLastSaved;
	if (l_oTitle.isEmpty()) l_oTitle = trUtf8("Untitled");

	int l_o = KMessageBox::warningYesNoCancel(NULL, //this,
	trUtf8("The document \"%1\" has been modified.\nDo you want to save your changes or discard them?").arg(l_oTitle),
	trUtf8("Close Document"),
	KStandardGuiItem::save(), KStandardGuiItem::discard());

	switch (l_o)
	{
		case KMessageBox::Yes:
			return slot_save();
		case KMessageBox::No :
			return true;
		default:
			return false;
	}
	return false;
}

void semantik_win::slot_tool_select()
{
	//qDebug()<<"slot tool select"<<endl;
	m_oCanvas->set_mode(canvas_view::select_mode);
}

void semantik_win::slot_tool_link()
{
	//qDebug()<<"slot tool link"<<endl;
	m_oCanvas->set_mode(canvas_view::link_mode);
}

void semantik_win::slot_tool_sort()
{
	//qDebug()<<"slot tool sort"<<endl;
	m_oCanvas->set_mode(canvas_view::sort_mode);
}

void semantik_win::slot_tool_scroll()
{
	//qDebug()<<"slot tool scroll"<<endl;
	m_oCanvas->set_mode(canvas_view::scroll_mode);
}

void semantik_win::slot_message(const QString & i_sMsg, int i_iDur)
{
	statusBar()->showMessage(i_sMsg, i_iDur);
}

void semantik_win::slot_recent(const KUrl& i_oUrl)
{
	if (i_oUrl.path().isEmpty()) return;
	if (m_oControl->open_file(i_oUrl.path()))
	{
		set_current_file(i_oUrl);
	}
}

void semantik_win::slot_tip_of_day()
{
	KTipDialog::showTip(this, notr("semantik/tips"), true);
}

#include "semantik.moc"
