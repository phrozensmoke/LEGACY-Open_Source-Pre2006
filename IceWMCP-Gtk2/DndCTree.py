################################################
## IceMe
# =====
#
# Copyright 2000-2002, Dirk Moebius <dmoebius@gmx.net> 
# and Mike Hostetler <thehaas@binary.net>
#
# This work comes with no warranty regarding its suitability for any purpose.
# The author is not responsible for any damages caused by the use of this
# program.
#
# This software is distributed under the GNU General Public License
#################################################
########
# With Modifications by Erica Andrews (PhrozenSmoke@yahoo.com), February-April 2003
# This is a modified version of IceMe 1.0.0 ("IceWMCP Edition"), optimized for IceWM ControlPanel.
# Copyright (c) 2003 Erica Andrews
#
# Ported to PyGTK-2 by: David Moore (djm6202@yahoo.co.nz) - July 2003
##################

import sys
import gtk

class DndCTree(gtk.CTree):

    COLUMN_TITLES_HEIGHT = 24

    def __init__(self, cols=1, tree_col=0, titles=None):
        gtk.CTree.__init__(self, cols, tree_col, titles)
        gtk.CTree.set_selection_mode(self, gtk.SELECTION_BROWSE)
        self.column_titles_height = self.COLUMN_TITLES_HEIGHT
        targets = [('application/x-gtk.CTreeNode',
                    gtk.TARGET_SAME_APP|gtk.TARGET_SAME_WIDGET, -1)]
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, targets, gtk.gdk.ACTION_MOVE)
        self.drag_dest_set(gtk.DEST_DEFAULT_ALL, targets, gtk.gdk.ACTION_MOVE)

        self.connect("drag-begin", self.cb_drag_begin)
        self.connect("drag-motion", self.cb_drag_motion)
        self.connect("drag-data-get", self.cb_drag_data_get)
        self.connect("drag-data-received", self.cb_drag_data_received)
        self.connect("drag-end", self.cb_drag_end)

    def cb_drag_data_get(self, widget, context, data, info, time):
        data.set(data.target, 1, "dummy-data")

    def cb_drag_begin(self, widget, context):
        if self.selection:
            self.the_drag_node = self.selection[0]

    def cb_drag_motion(self, widget, context, x, y, time):
        selected = self.get_selection_info(x, y - self.column_titles_height)
        if selected:
            row = selected[0]
            self.select_row(row, 0)
            visibility = self.row_is_visible(row)
            if visibility != gtk.VISIBILITY_FULL:
                visibility_next = self.row_is_visible(row + 1)
                if visibility_next == gtk.VISIBILITY_NONE:
                    self.moveto(row, 0, 1.0, 0.0)
                else:
                    self.moveto(row, 0, 0.0, 0.0)

    def cb_drag_data_received(self, widget, context, x, y, data, info, time):
        selected = self.get_selection_info(x, y - self.column_titles_height)
        if selected:
            target_node = self.node_nth(selected[0])
            # move self.the_drag_node under target_node
            if target_node == self.the_drag_node:
                # source = target: no move neccessary
                return
            if target_node.is_leaf:
                new_parent = target_node.parent
                new_sibling = target_node.sibling
            else:
                new_parent = target_node
                if target_node.children:
                    new_sibling = target_node.children[0]
                else:
                    new_sibling = None
            if new_sibling == self.the_drag_node:
                # move under direct predecessor: no move neccessary
                return
            if not self.move_is_permitted(self.the_drag_node,
                                          target_node,
                                          new_parent,
                                          new_sibling):
                return
            self.move(self.the_drag_node, new_parent, new_sibling)
            if not target_node.is_leaf:
                self.expand(target_node)

    def cb_drag_end(self, widget, context):
        self.the_drag_node = None

    def set_selection_mode(self, mode):
        if mode != gtk.SELECTION_BROWSE:
            sys.stderr.write("*** DndCTree.set_selection_mode:" \
                             " only supports SELECTION_BROWSE\n")

    def column_titles_show(self):
        gtk.Tree.column_titles_show(self)
        self.column_titles_height = self.COLUMN_TITLES_HEIGHT

    def column_titles_hide(self):
        gtk.Tree.column_titles_hide(self)
        self.column_titles_height = 0

    def move_is_permitted(self, source_node, target_node,
                          new_parent, new_sibling):
        """Tests, whether source_node may be moved under target_node,
        getting new_parent and new_sibling as its insert position.

        This is invoked after an drag gesture, before the actual move
        operation takes place. The default implementation always returns 1.
        Subclasses may override this to control the permissable drag
        operations.
        """
        return 1


DnDCTree = DndCTree



def test():
    tree = DndCTree(cols=2, titles=["Tree", "Type"] )
    tree.set_column_auto_resize(0, gtk.TRUE)
    n1  = tree.insert_node(None, None, ["n1" , "menu"]   , is_leaf=gtk.FALSE)
    n2  = tree.insert_node(n1,   None, ["n2" , "leaf"])
    n3  = tree.insert_node(n1,   None, ["n3" , "leaf"])
    n4  = tree.insert_node(None, None, ["n4" , "menu"]   , is_leaf=gtk.FALSE)
    n5  = tree.insert_node(None, None, ["n5" , "menu"]   , is_leaf=gtk.FALSE)
    n6  = tree.insert_node(None, None, ["n6" , "menu"]   , is_leaf=gtk.FALSE)
    n7  = tree.insert_node(n6,   None, ["n7" , "submenu"], is_leaf=gtk.FALSE)
    n8  = tree.insert_node(n7,   None, ["n8" , "submenu"], is_leaf=gtk.FALSE)
    n9  = tree.insert_node(n8,   None, ["n9" , "leaf"])
    n10 = tree.insert_node(None, None, ["n10", "leaf"])
    n11 = tree.insert_node(None, None, ["n11", "leaf"])
    n12 = tree.insert_node(None, None, ["n12", "leaf"])
    n13 = tree.insert_node(None, None, ["n13", "leaf"])
    tree.expand_recursive(n1)
    tree.expand_recursive(n6)
    tree.show()

    scrolled_win = gtk.ScrolledWindow()
    scrolled_win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
    scrolled_win.add(tree)
    scrolled_win.show()

    window = gtk.Window()
    window.set_default_size(300, 300)
    window.connect("delete_event", gtk.mainquit)
    window.add(scrolled_win)
    window.show()

    gtk.mainloop()

if __name__ == "__main__":
    test()




