/*
 * Vdownloader Extension - Popup Logic
 */

document.addEventListener("DOMContentLoaded", () => {
    const enableToggle = document.getElementById("enableToggle");
    const interceptToggle = document.getElementById("interceptToggle");
    const urlInput = document.getElementById("urlInput");
    const downloadBtn = document.getElementById("downloadBtn");
    const statusDot = document.getElementById("statusDot");
    const connectionStatus = document.getElementById("connectionStatus");

    // Get current status
    chrome.runtime.sendMessage({ action: "getStatus" }, (response) => {
        if (response) {
            enableToggle.checked = response.enabled;
            interceptToggle.checked = response.interceptDownloads;
            updateConnectionUI(response.connected);
        }
    });

    function updateConnectionUI(connected) {
        if (connected) {
            statusDot.classList.add("connected");
            connectionStatus.textContent = "● Connected";
            connectionStatus.classList.add("connected");
        } else {
            statusDot.classList.remove("connected");
            connectionStatus.textContent = "● Disconnected";
            connectionStatus.classList.remove("connected");
        }
    }

    // Toggle handlers
    enableToggle.addEventListener("change", () => {
        chrome.runtime.sendMessage(
            { action: "toggle", enabled: enableToggle.checked },
            (response) => {
                if (response) enableToggle.checked = response.enabled;
            }
        );
    });

    interceptToggle.addEventListener("change", () => {
        chrome.runtime.sendMessage(
            { action: "toggleIntercept", intercept: interceptToggle.checked },
            (response) => {
                if (response) interceptToggle.checked = response.interceptDownloads;
            }
        );
    });

    // Download button
    downloadBtn.addEventListener("click", () => {
        const url = urlInput.value.trim();
        if (!url) return;

        // Basic URL validation
        if (!url.match(/^(https?|ftp|magnet):/i)) {
            urlInput.style.borderColor = "#ef4444";
            setTimeout(() => (urlInput.style.borderColor = ""), 1500);
            return;
        }

        downloadBtn.disabled = true;
        downloadBtn.textContent = "Sending...";

        chrome.runtime.sendMessage(
            {
                action: "downloadUrl",
                url: url,
                filename: url.split("/").pop().split("?")[0] || "download",
                referrer: "",
            },
            (response) => {
                if (response && response.success) {
                    downloadBtn.innerHTML = '<span>✓</span> Sent!';
                    downloadBtn.style.background =
                        "linear-gradient(135deg, #10b981, #059669)";
                    urlInput.value = "";
                } else {
                    downloadBtn.innerHTML = '<span>✗</span> Failed';
                    downloadBtn.style.background =
                        "linear-gradient(135deg, #ef4444, #dc2626)";
                }
                setTimeout(() => {
                    downloadBtn.disabled = false;
                    downloadBtn.innerHTML = '<span>⬇</span> Download';
                    downloadBtn.style.background = "";
                }, 2000);
            }
        );
    });

    // Enter key to download
    urlInput.addEventListener("keydown", (e) => {
        if (e.key === "Enter") downloadBtn.click();
    });
});
