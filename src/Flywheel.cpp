namespace Flywheel {

    bool active = false;

    bool SpinUp() {
        active = true;
    }

    bool Stop() {
        active = false;
    }
}