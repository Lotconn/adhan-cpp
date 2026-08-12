function buildMatrix() {

    return {

        summary: null,

        selected: null,

        log: "",

        search: "",

        // Width in pixels of the left (case list) panel; dragged via the
        // divider between the case list and the log/details panel.
        leftWidth: 320,

        resizing: false,

        async initialize() {

            // index.html lives in html/, matrix.json and cases/ are one
            // directory up (see config.py for the on-disk layout).
            this.summary = await fetch("../matrix.json")
                .then(response => response.json());

            if (this.summary.cases.length > 0) {
                await this.select(
                    this.summary.cases[0]
                );
            }

            window.addEventListener("mousemove", (event) => this.onResize(event));
            window.addEventListener("mouseup", () => this.stopResize());

        },

        errorCount() {

            if (this.summary === null) {
                return 0;
            }

            return this.summary.cases.filter(
                entry => entry.error
            ).length;

        },

        statusLabel(entry) {

            if (entry.error) {
                return "ERROR";
            }

            return entry.success ? "PASS" : "FAIL";

        },

        badgeClasses(entry) {

            if (entry.error) {
                return "bg-amber-100 text-amber-800";
            }

            return entry.success
                ? "bg-green-100 text-green-700"
                : "bg-red-100 text-red-700";

        },

        caseClasses(entry) {

            const isSelected = (
                this.selected &&
                this.selected.name === entry.name
            );

            const base = isSelected
                ? "bg-blue-50 border-l-blue-600"
                : "border-l-transparent hover:bg-slate-50";

            if (entry.error) {
                return `${base} ${isSelected ? "" : "bg-amber-50/60"}`;
            }

            if (entry.success) {
                return `${base} ${isSelected ? "" : "bg-green-50/60"}`;
            }

            return `${base} ${isSelected ? "" : "bg-red-50/60"}`;

        },

        startResize() {

            this.resizing = true;

        },

        onResize(event) {

            if (!this.resizing) {
                return;
            }

            const min = 220;
            const max = 560;

            this.leftWidth = Math.min(
                max,
                Math.max(min, event.clientX - 24)
            );

        },

        stopResize() {

            this.resizing = false;

        },

        filteredCases() {

            if (this.summary === null) {
                return [];
            }

            const search = this.search
                .trim()
                .toLowerCase();

            if (search.length === 0) {
                return this.summary.cases;
            }

            return this.summary.cases.filter(
                entry =>
                    entry.name
                        .toLowerCase()
                        .includes(search)
            );

        },

        async select(caseSummary) {

            const details = await fetch(
                "../cases/" +
                caseSummary.name +
                ".json"
            ).then(
                response => response.json()
            );

            this.selected = details;

            try {

                this.log = await fetch(
                    "../" +
                    details.log_file
                ).then(
                    response => response.text()
                );

            } catch {

                this.log =
                    "Unable to load log.";

            }

        },

        buildSteps() {

            if (this.selected === null) {
                return [];
            }

            return [

                this.step(
                    "Configure",
                    this.selected.configure
                ),

                this.step(
                    "Build",
                    this.selected.build
                ),

                this.step(
                    "Tests",
                    this.selected.tests
                ),

                this.step(
                    "Install",
                    this.selected.install
                ),

            ];

        },

        step(
            label,
            data
        ) {

            if (
                data === null ||
                data === undefined
            ) {

                return {
                    name: label,
                    label: label,
                    status: "Skipped",
                    time: "",
                };

            }

            return {
                name: label,
                label: label,
                status: data.success
                    ? "PASS"
                    : "FAIL",
                time: this.formatSeconds(
                    data.elapsed
                ),
            };

        },

        formatSeconds(
            seconds
        ) {

            if (
                seconds === null ||
                seconds === undefined
            ) {
                return "";
            }

            return (
                seconds.toFixed(3)
                + " s"
            );

        },

        async copyLog() {

            await navigator.clipboard.writeText(
                this.log
            );

        },

    };

}