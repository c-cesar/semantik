// Thomas Nagy 2013 GPLV3

#include <QtGui>
#include <QFile>
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
#include <QFrame>

#include "semantik_d_win.h"
#include "sem_mediator.h"
#include "box_view.h"
# include  "sembind.h"

semantik_d_win::semantik_d_win(QWidget *i_oParent) : KXmlGuiWindow(i_oParent)
{
	setObjectName("semantik_win");

	setWindowIcon(KIcon("semantik"));

	m_oMediator = new sem_mediator(this);

	QFrame *fr = new QFrame(this);
        fr->setLineWidth(0);
        fr->setFrameStyle(QFrame::NoFrame);
        m_oDiagramView = new box_view(fr, m_oMediator);
        setCentralWidget(fr);
        QGridLayout *ll = new QGridLayout(fr);
        ll->addWidget(m_oDiagramView);

	KStandardAction::quit(this, SLOT(close()), actionCollection());
	KStandardAction::save(this, SLOT(slot_save()), actionCollection());
	KStandardAction::saveAs(this, SLOT(slot_save_as()), actionCollection());
	KStandardAction::open(this, SLOT(slot_open()), actionCollection());
	KStandardAction::print(this, SLOT(slot_print()), actionCollection());
	KStandardAction::tipOfDay(this, SLOT(slot_tip_of_day()), actionCollection());
	m_oUndoAct = KStandardAction::undo(m_oMediator, SLOT(slot_undo()), actionCollection());
	m_oUndoAct->setEnabled(false);
	m_oRedoAct = KStandardAction::redo(m_oMediator, SLOT(slot_redo()), actionCollection());
	m_oRedoAct->setEnabled(false);

	m_oRecentFilesAct = KStandardAction::openRecent(this, SLOT(slot_recent(const KUrl&)), actionCollection());


        setupGUI(QSize(800, 800), Default, notr("semantik/semantik-dui.rc"));

	read_config();
	statusBar()->showMessage(trUtf8("Welcome to Semantik"), 2000);
	setAutoSaveSettings();
}

semantik_d_win::~semantik_d_win()
{

}

void semantik_d_win::read_config()
{
	KConfigGroup l_oConfig(KGlobal::config(), notr("General Options"));
	m_oRecentFilesAct->loadEntries(KGlobal::config()->group(notr("Recent Files")));
	move(l_oConfig.readEntry(notr("winpos"), QPoint(0, 0)));
	m_oMediator->m_sOutDir = l_oConfig.readEntry(notr("outdir"), notr("/tmp/"));
	bind_node::set_var(notr("outdir"), m_oMediator->m_sOutDir);
}

void semantik_d_win::write_config()
{
	KConfigGroup l_oConfig(KGlobal::config(), notr("General Options"));
	m_oRecentFilesAct->saveEntries(KGlobal::config()->group(notr("Recent Files")));
	l_oConfig.writeEntry(notr("winpos"), pos());
	l_oConfig.writeEntry(notr("outdir"), bind_node::get_var(notr("outdir")));
	l_oConfig.sync();
}

bool semantik_d_win::queryClose()
{
	write_config();
	//if (!m_oMediator->m_bDirty) return true;
	//return proceed_save();
	return true;
}

void semantik_d_win::update_title() {
}

void semantik_d_win::slot_open() {
}

bool semantik_d_win::slot_save() {
	return true;
}

bool semantik_d_win::slot_save_as() {
	return true;
}

void semantik_d_win::slot_print() {
}

void semantik_d_win::slot_recent(const KUrl &) {
}

void semantik_d_win::slot_enable_undo(bool, bool) {
}

void semantik_d_win::slot_properties() {
}

void semantik_d_win::slot_generate() {
}

void semantik_d_win::slot_tip_of_day() {
}

void semantik_d_win::slot_message(const QString &, int) {
}

#include "semantik_d_win.moc"

