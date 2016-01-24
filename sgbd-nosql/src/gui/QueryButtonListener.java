package gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.JComboBox;
import javax.swing.JLabel;

import entrepot.OlapArbre;
import entrepot.OlapRequete.Operator;

/**
 * listener pour les bouton contenu dans QueryPanel
 * @author adel
 *
 */
public class QueryButtonListener implements ActionListener {

    private TreeSelectPanel treeP;
    private int selectionType;
    private JLabel lab;
    public static final int UNIQUE = 1;
    public static final int MULTIPLE = 2;
    private JComboBox combo;
    public ArrayList<OlapArbre> selected;

    /**
	 * @param selected the selected to set
	 */
	public void setSelected(ArrayList<OlapArbre> selected) {
		this.lab.setText("-");
		this.selected = selected;
	}

	public QueryButtonListener(TreeSelectPanel treeP, JLabel lab,
            JComboBox combo) {
        this(treeP, lab, combo, MULTIPLE);
    }

    /**
     *  constructeur de la classe
     * @param treeP
     * 			JPanel qui contient l'arbre olap (pour recup�rer l'id des noeud selectionn�
     * @param lab
     * 			JLabel associer au bouton (affichera les noeud selectionner)
     * @param combo
     * 			si !null alors le JComboBox contiendra toute les operations applicable au noeud selectionner
     * @param selectionType
     *   possibilit� de selectionner un  ou plusieurs noeud
     */
    public QueryButtonListener(TreeSelectPanel treeP, JLabel lab,
            JComboBox combo, int selectionType) {
        this.treeP = treeP;
        this.combo = combo;
        this.selectionType = selectionType;
        this.lab = lab;
    }

    @Override
    public void actionPerformed(ActionEvent arg0) {
        selected = treeP.getSelected();
        lab.setText(selected.toString());
        if (selectionType == QueryButtonListener.UNIQUE && selected.size() != 1) {
            lab.setText("");
            selected = null;
        }
        if (combo != null) {
            Operator[] opList = Operator.getListeOperateur(null);
            if (selected == null) {
                System.out.println("here");
                opList = Operator.getListeOperateur(null);
                combo.removeAllItems();

            } else {
                System.out.println("h2ere");
                opList = Operator.getListeOperateur(selected.get(0));
                combo.removeAllItems();

            }
            for (Operator op : opList) {
                combo.addItem(op);
            }
        }
    }

    /**
     * @return les noeuds selectionner
     */
    public ArrayList<OlapArbre> getSelected() {
        return selected;
    }
}
