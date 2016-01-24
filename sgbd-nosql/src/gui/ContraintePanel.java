package gui;

import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;

import javax.swing.*;
import javax.swing.event.*;

import entrepot.Contrainte;
import entrepot.OlapArbre;

/**
 * JPanel pour la cr�ation de contraintes
 * prend en parametre  un  TreeSelectPanel pour trouver ID du noeud selectionn�
 * @author adel
 *
 */
public class ContraintePanel extends JPanel implements ListSelectionListener {

    /**
     *
     */
    private static final long serialVersionUID = 1L;
    private JList list;
    private DefaultListModel listModel;
    private static final String hireString = "add";
    private static final String fireString = "remove";
    private JButton fireButton;
    private JTextField contrainteValue;
    private TreeSelectPanel tsp;
    private JTextField operateurValue;

    /**
     *
     * @return la liste des contrainte creer par l'utilisateur
     */
    public ArrayList<Contrainte> getContrainte() {
        ArrayList<Contrainte> contrainte = new ArrayList<Contrainte>();
        for (int i = 0; i < listModel.getSize(); i++) {
            String scont = (String) listModel.get(i);
            String[] t = scont.split(",");
            contrainte.add(new Contrainte(Integer.parseInt(t[1]), t[2], t[3]));
        }
        return contrainte;


    }

    /**
     *
     * @param tsp un  TreeSelectPanel pour trouver l'ID du noeud selectionn�
     */
    public ContraintePanel(TreeSelectPanel tsp) {
        super(new BorderLayout());
        this.tsp = tsp;
        listModel = new DefaultListModel();
        // Create the list and put it in a scroll pane.
        list = new JList(listModel);
        list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        list.addListSelectionListener(this);
        list.setVisibleRowCount(5);
        JScrollPane listScrollPane = new JScrollPane(list);

        JButton hireButton = new JButton(hireString);
        HireListener hireListener = new HireListener(hireButton);
        hireButton.setActionCommand(hireString);
        hireButton.addActionListener(hireListener);
        hireButton.setEnabled(false);

        fireButton = new JButton(fireString);
        fireButton.setActionCommand(fireString);
        fireButton.addActionListener(new FireListener());

        contrainteValue = new JTextField(10);
        contrainteValue.addActionListener(hireListener);
        contrainteValue.getDocument().addDocumentListener(hireListener);
        operateurValue = new JTextField(4);
        operateurValue.addActionListener(hireListener);
        operateurValue.getDocument().addDocumentListener(hireListener);
        // Create a panel that uses BoxLayout.
        JPanel buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.add(fireButton);
        buttonPane.add(Box.createHorizontalStrut(5));
        buttonPane.add(new JSeparator(SwingConstants.VERTICAL));
        buttonPane.add(operateurValue);
        buttonPane.add(Box.createHorizontalStrut(5));
        buttonPane.add(contrainteValue);
        buttonPane.add(hireButton);
        buttonPane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

        add(listScrollPane, BorderLayout.CENTER);
        add(buttonPane, BorderLayout.PAGE_END);
    }

    /**
     * listner pour les 2 boutons du panel
     * @author adel
     *
     */
    class FireListener implements ActionListener {

        public void actionPerformed(ActionEvent e) {
            int index = list.getSelectedIndex();
            listModel.remove(index);

            int size = listModel.getSize();

            if (size == 0) {
                fireButton.setEnabled(false);

            } else {
                if (index == listModel.getSize()) {

                    index--;
                }

                list.setSelectedIndex(index);
                list.ensureIndexIsVisible(index);
            }
        }
    }

    /**
     * listner pour les 2 boutons du panel
     * @author adel
     *
     */
    class HireListener implements ActionListener, DocumentListener {

        private boolean alreadyEnabled = false;
        private JButton button;

        public HireListener(JButton button) {
            this.button = button;
        }

        // Required by ActionListener.
        public void actionPerformed(ActionEvent e) {
            String name = contrainteValue.getText();
            String operateur = operateurValue.getText();
            // User didn't type in a unique name...
            if (name.equals("") || alreadyInList(name) || operateur.equals("")) {
                Toolkit.getDefaultToolkit().beep();
                contrainteValue.requestFocusInWindow();
                contrainteValue.selectAll();
                return;
            }
            if (operateur.equals("")) {
                Toolkit.getDefaultToolkit().beep();
                operateurValue.requestFocusInWindow();
                operateurValue.selectAll();
                return;
            }

            int index = list.getSelectedIndex(); // get selected index
            if (index == -1) { // no selection, so insert at beginning
                index = 0;
            } else { // add after the selected item
                index++;
            }
            String contrainte;
            ArrayList<OlapArbre> selected = tsp.getSelected();
            if (selected.size() == 1) {
                contrainte = selected.get(0).toString() + ',' + selected.get(0).getId() + "," + operateur + ","
                        + name;
                listModel.insertElementAt(contrainte, index);
                // If we just wanted to add to the end, we'd do this:
                // listModel.addElement(contrainteValue.getText());
            }

            // Reset the text field.
            contrainteValue.requestFocusInWindow();
            contrainteValue.setText("");
            operateurValue.setText("");
            // Select the new item and make it visible.
            list.setSelectedIndex(index);
            list.ensureIndexIsVisible(index);
        }

        // This method tests for string equality. You could certainly
        // get more sophisticated about the algorithm. For example,
        // you might want to ignore white space and capitalization.
        protected boolean alreadyInList(String name) {
            return listModel.contains(name);
        }

        // Required by DocumentListener.
        public void insertUpdate(DocumentEvent e) {
            enableButton();
        }

        // Required by DocumentListener.
        public void removeUpdate(DocumentEvent e) {
            handleEmptyTextField(e);
        }

        // Required by DocumentListener.
        public void changedUpdate(DocumentEvent e) {
            if (!handleEmptyTextField(e)) {
                enableButton();
            }
        }

        private void enableButton() {
            if (!alreadyEnabled && !contrainteValue.getText().equals("") && !operateurValue.getText().equals("")) {

                button.setEnabled(true);

            }
        }

        private boolean handleEmptyTextField(DocumentEvent e) {
            if (e.getDocument().getLength() <= 0) {
                button.setEnabled(false);
                alreadyEnabled = false;
                return true;
            }
            return false;
        }
    }

    /**
     * @return the listModel
     */
    public DefaultListModel getListModel() {
        return listModel;
    }

    public void valueChanged(ListSelectionEvent e) {
        if (e.getValueIsAdjusting() == false) {

            if (list.getSelectedIndex() == -1) {
                // No selection, disable fire button.
                fireButton.setEnabled(false);

            } else {
                // Selection, enable the fire button.
                fireButton.setEnabled(true);
            }
        }
    }
}
