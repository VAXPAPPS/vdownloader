/*
 * Vdownloader Content Script
 * Detects downloadable links and adds visual indicators.
 */

(function () {
    "use strict";

    const DOWNLOAD_EXTENSIONS = [
        // Archives
        "zip", "rar", "7z", "tar", "gz", "bz2", "xz", "iso", "dmg",
        // Media
        "mp4", "mkv", "avi", "mov", "wmv", "flv", "webm", "mp3", "flac",
        "wav", "aac", "ogg", "m4a",
        // Documents
        "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx",
        // Software
        "exe", "msi", "deb", "rpm", "appimage", "snap", "flatpak",
        // Images
        "png", "jpg", "jpeg", "gif", "bmp", "svg", "webp", "tiff",
        // Torrents
        "torrent",
    ];

    function getExtension(url) {
        try {
            const pathname = new URL(url).pathname;
            const ext = pathname.split(".").pop().toLowerCase();
            return ext.length <= 10 ? ext : "";
        } catch {
            return "";
        }
    }

    function isDownloadLink(url) {
        const ext = getExtension(url);
        return DOWNLOAD_EXTENSIONS.includes(ext);
    }

    // Listen for right-click on download links to provide info to background
    document.addEventListener("contextmenu", (e) => {
        const link = e.target.closest("a[href]");
        if (link && isDownloadLink(link.href)) {
            // Store the link info for potential use
            chrome.runtime.sendMessage({
                action: "linkDetected",
                url: link.href,
                text: link.textContent.trim(),
                referrer: window.location.href,
            });
        }
    });

    // Intercept clicks on download links
    document.addEventListener("click", (e) => {
        const link = e.target.closest("a[href]");
        if (!link) return;

        const url = link.href;
        if (!isDownloadLink(url)) return;

        // Check if download attribute is set or link is direct file
        if (link.hasAttribute("download") || getExtension(url)) {
            e.preventDefault();
            e.stopPropagation();

            const filename =
                link.getAttribute("download") ||
                url.split("/").pop().split("?")[0] ||
                "download";

            chrome.runtime.sendMessage({
                action: "downloadUrl",
                url: url,
                filename: filename,
                referrer: window.location.href,
            });
        }
    }, true);
})();
