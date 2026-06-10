#pragma once

#include <QString>

class CommandLineRunner {
public:
    // Run a single workflow, optionally overriding input/output paths
    // Returns 0 on success, 1 on error
    static int run(const QString &workflowPath,
                   const QString &imagePath = QString(),
                   const QString &outputPath = QString(),
                   QString *errorMsg = nullptr);
};
