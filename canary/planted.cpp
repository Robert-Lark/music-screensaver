// The canary: one planted violation for the strict compiler flags (an unused variable).
int main() {
    int unused = 0;
    return 0;
}
