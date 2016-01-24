package gui;

import java.awt.GridLayout;
import java.awt.Toolkit;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;

import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import entrepot.OlapArbre;
import entrepot.OlapArbre.NomExistantException;
import entrepot.SQLType;

/**
 * Jpanel de cr�ation d'un arbreOlap (contenu dans la classe CreateTreeFrame)
 * @author adel
 *
 */
public class CreateTreePanel extends JPanel {

    /**
     *
     */
    private static final long serialVersionUID = 1L;
    protected OlapArbre treeModel;
    protected JTree tree;
    private Toolkit toolkit = Toolkit.getDefaultToolkit();

    public CreateTreePanel() {
        super(new GridLayout(1, 0));

        try {
            treeModel = new OlapArbre("O", SQLType.VARCHAR);
        } catch (NomExistantException e) {

            e.printStackTrace();
        }

        // treeModel.addTreeModelListener(new MyTreeModelListener());
        tree = new JTree(treeModel);
        tree.setEditable(true);
        tree.getSelectionModel().setSelectionMode(
                TreeSelectionModel.SINGLE_TREE_SELECTION);
        tree.setShowsRootHandles(true);

        JScrollPane scrollPane = new JScrollPane(tree);
        add(scrollPane);
    }

    public OlapArbre getTreeModel() {
        return treeModel;
    }

    /** Remove the currently selected node. */
    public void removeCurrentNode() {
        TreePath currentSelection = tree.getSelectionPath();
        if (currentSelection != null) {
            OlapArbre currentNode = (OlapArbre) currentSelection.getLastPathComponent();
            if (!currentNode.isRacine()) {
                treeModel.supprimerFils(currentNode.getId());
                tree.updateUI();
                return;
            }
        }
        tree.updateUI();
        toolkit.beep();

    }

    /** Add child to the currently selected node. */
    public void addObject(OlapArbre child) {
        OlapArbre parentNode = null;
        TreePath parentPath = tree.getSelectionPath();

        if (parentPath == null) {
            parentNode = treeModel;
        } else {
            parentNode = (OlapArbre) (parentPath.getLastPathComponent());
        }
        parentNode.ajouterFils(child);
        tree.updateUI();
    }
}
