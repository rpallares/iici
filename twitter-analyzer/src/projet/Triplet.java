package projet;

public class Triplet<X, Y, Z> {

    private X first;
    private Y second;
    private Z third;

    public Triplet(X x, Y y, Z z) {
        first = x;
        second = y;
        third = z;
    }

    public X getFirst() {
        return first;
    }

    public Y getSecond() {
        return second;
    }

    public Z getThird() {
        return third;
    }

    public void setThird(Z z) {
        third = z;
    }
}
