#sem:name: Beamer presentation
#sem:tip: Generates a LaTeX presentation using Beamer

# Thomas Nagy, 2007 (ita)
import os, shutil, time, StringIO
from sgmllib import SGMLParser
import htmlentitydefs

outdir = sembind.get_var('outdir')+'/'+sembind.get_var('pname')

settings = {
'doc_date':'',
'doc_content':'',
'doc_title':'How i did it',
'doc_author':'Krusty',
'doc_place':'Los angeles',
'doc_company':'World company',

'theme':'Warsaw',
'babel':'english', # frenchb
#'doc_tableofcontents_off':''
'content_title':'Contents',

'doc_title':'',
'doc_title_off':'None',

'doc_date':r'\today',
'doc_date_off':'None',

'doc_author':'',
'doc_author_off':'None',

'each_subsection_off':'None',
'doc_tableofcontents_off':'None',
}
add_globals(settings)

# do the document in latex ?
def tex_convert(s):
	return s
if not settings.get('all_latex', 0): tex_convert = protect_tex

try:
	tm = time.strftime('.%d-%b-%y--%H-%M-%S', time.gmtime(os.stat(outdir).st_mtime))
	os.rename(outdir, outdir+tm)
except OSError:
	pass

try: os.makedirs(outdir)
except OSError: debug('Cannot create folder ' + outdir)

# copy the pictures
cwd = os.getcwd()
os.chdir(sembind.get_var('temp_dir'))
pics = {} # map the id to the picture
lst = os.listdir('.')
for i in lst:
	if -1 == i.rfind('pic-'): continue
	pics[ i.replace('pic-', '').split('.')[0] ] = i
	shutil.copy2(i, outdir)
os.chdir(cwd)


buf = []
out = buf.append

class TrucProcessor(SGMLParser):
	def reset(self):
		self.pieces = []
		self.state = ""
		self.buf = ""
		self.inli = 0
		SGMLParser.reset(self)

	def unknown_starttag(self, tag, attrs):
		if tag == 'ul':
			if self.inli and self.buf:
				self.pieces.append('\\item ')
				self.pieces.append(self.buf)
				self.pieces.append('\n')
			self.pieces.append('\\begin{itemize}\n')

		if tag == 'li':
			self.inli += 1

		self.buf = ""

	def unknown_endtag(self, tag):
		if tag == 'p':
			self.pieces.append(self.buf)
			self.pieces.append('\n')
		elif tag == 'li':
			if self.buf:
				self.pieces.append('\\item ')
				self.pieces.append(self.buf)
				self.pieces.append('\n')
			self.inli -= 1
		elif tag == 'ul':
			self.pieces.append('\\end{itemize}\n')

	def handle_charref(self, ref):
		self.pieces.append("&#%(ref)s;" % locals())

	def handle_data(self, text):
		self.buf = text

	def output(self):
		return "".join(self.pieces)

def print_slide(node, niv):
	txt = tex_convert(node.get_val('summary'))
	if niv == 0:
		out('%-------------------------------------------------------------------\n')
		out('\\begin{frame}\n')
		out('\\frametitle{%s}\n\n' % txt)
		num = node.child_count()
		if num:
			out("\\begin{itemize}\n")
			for i in range(num):
				print_slide(node.child_num(i), niv+1)
			out("\\end{itemize}\n")
		out('\\end{frame}\n')
		out('%-------------------------------------------------------------------\n')
	elif niv < 3:
		if txt: out('\\item %s\n' % txt)
		num = node.child_count()
		if num:
			out("\\begin{itemize}\n")
			for i in range(num):
				print_slide(node.child_num(i), niv+1)
			out("\\end{itemize}\n")
	elif niv == 3:
		if txt: out('\\item %s\n' % txt)
		num = node.child_count()
		for i in range(num):
			print_slide(node.child_num(i), niv+1)

	else:
		if txt: out('%s %s\n' % ('%', txt))
		num = node.child_count()
		for i in range(num):
			print_slide(node.child_num(i), niv+1)

def print_nodes(node, niv):

	num = node.child_count()
	for i in range(num):
		subtree = node.child_num(i)
		#print_nodes(node.child_num(i), niv+1)

		sm = tex_convert(subtree.get_val('summary'))
		num = subtree.child_count()
		if niv == 0 and num >= 1:
			out('\\section{%s}\n' % sm)
		elif niv == 1 and num >= 1:
			out('\\subsection{%s}\n' % sm)

		if int(subtree.get_val('tree_size')) < 10:
			print_slide(subtree, 0);
		elif niv == 0:
			print_nodes(subtree, 1)
		elif niv == 1:
			print_nodes(subtree, 2)
		else:
			sys.stderr.write("transforming this map into slides makes kitten cry")

#	rows = node.num_rows()
#	cols = node.num_cols()
#	if rows>0 and cols>0:
#		out('\n')
#		out('<table class="sem_table" cellspacing="0px" cellpadding="0px">\n')
#		out('<tbody>\n')
#		for i in range(rows):
#			out('\t<tr>\n')
#			for j in range(cols):
#				if i>0 and j>0:
#					out('\t\t<td>%s</td>\n' % x(node.get_cell(i, j)))
#				else:
#					out('\t\t<th>%s</th>\n' % x(node.get_cell(i, j)))
#			out('\t</tr>\n')
#
#		out('</tbody>\n')
#		out('</table>\n')
#	out('\n')

# the main document
print_nodes(Root(), 0);
settings['doc_content'] = "".join(buf)

# now write main.tex
transform("/beamer/main.tex", outdir+'/main.tex', settings)

# data files

os.popen('cp -Rf %s %s' % (template_dir()+'/beamer/beamermindist/', outdir)).read()

# anciliary files
shutil.copy2(template_dir()+'/beamer/wscript', outdir+'/wscript')
shutil.copy2(template_dir()+'/waf', outdir+'/waf')
os.chmod(outdir+'/waf', 0755)

# load the preview on main.tex
visualize('beamer', outdir+'/main.tex')


