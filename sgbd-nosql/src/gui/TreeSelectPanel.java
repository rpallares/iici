package gui;

import java.awt.BorderLayout;
import java.util.ArrayList;


import javax.swing.JPanel;
import javax.swing.JTree;

import javax.swing.tree.TreePath;


import entrepot.OlapArbre;

/**
 * JPanel qui affiche l'arbre Olap et qui retourne les element selectionner
 * @author adel
 *
 */
public class TreeSelectPanel extends JPanel {

    /**
     *
     */
    private static final long serialVersionUID = 3L;
    private JTree tree;

    public TreeSelectPanel(OlapArbre treeMod) {
        super();
        this.setLayout(new BorderLayout());
        tree = new JTree(treeMod);
        for (int i = 0; i < tree.getRowCount(); i++) {
            tree.expandRow(i);
        }
        // tree.addTreeSelectionListener(new EcouteurOlapTree(tree));
        this.add(tree);
    }

    public ArrayList<OlapArbre> getSelected() {
        TreePath[] paths = tree.getSelectionPaths();
        ArrayList<OlapArbre> tmp = new ArrayList<OlapArbre>();
        if (paths != null) {
            for (int j = 0; j < paths.length; j++) {
                tmp.add((OlapArbre) paths[j].getLastPathComponent());
                //System.out.print(a.getId() + " ");
            }
        }
        return tmp;
    }
}
