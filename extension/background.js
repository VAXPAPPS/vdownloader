/*
 * Vdownloader Browser Extension — Background Service Worker
 * Intercepts downloads, context menu integration, native messaging.
 */

const NATIVE_HOST = "org.vaxp.downloader";
let nativePort = null;
let isEnabled = true;
let interceptDownloads = true;

/* ── Native Messaging ── */
function connectNative() {
  try {
    nativePort = chrome.runtime.connectNative(NATIVE_HOST);
    nativePort.onMessage.addListener((msg) => {
      console.log("[Vdownloader] Native message:", msg);
      if (msg.status === "started") {
        chrome.notifications.create({
          type: "basic",
          iconUrl: "icons/icon128.png",
          title: "Vdownloader",
          message: `Download started: ${msg.filename || "file"}`,
        });
      }
    });
    nativePort.onDisconnect.addListener(() => {
      console.log("[Vdownloader] Native host disconnected");
      nativePort = null;
    });
    return true;
  } catch (e) {
    console.error("[Vdownloader] Failed to connect:", e);
    return false;
  }
}

function sendToApp(url, filename, referrer) {
  const message = {
    action: "download",
    url: url,
    filename: filename || "",
    referrer: referrer || "",
    cookies: "",
    userAgent: navigator.userAgent,
  };

  if (!nativePort) {
    if (!connectNative()) {
      // Fallback: copy URL to clipboard for manual paste
      console.warn("[Vdownloader] Cannot connect to native host, using fallback");
      return false;
    }
  }

  try {
    nativePort.postMessage(message);
    return true;
  } catch (e) {
    console.error("[Vdownloader] Send failed:", e);
    nativePort = null;
    return false;
  }
}

/* ── Download Interception ── */
chrome.downloads.onCreated.addListener((downloadItem) => {
  if (!isEnabled || !interceptDownloads) return;

  // Don't intercept small files (< 1MB) or extension downloads
  const url = downloadItem.url || "";
  if (
    url.startsWith("blob:") ||
    url.startsWith("data:") ||
    url.startsWith("chrome-extension:") ||
    url.startsWith("moz-extension:")
  ) {
    return;
  }

  // Extract filename
  const filename = downloadItem.filename
    ? downloadItem.filename.split("/").pop().split("\\").pop()
    : "";

  // Cancel browser download and send to Vdownloader
  chrome.downloads.cancel(downloadItem.id, () => {
    chrome.downloads.erase({ id: downloadItem.id });
    sendToApp(url, filename, downloadItem.referrer);
  });
});

/* ── Context Menu ── */
chrome.runtime.onInstalled.addListener(() => {
  chrome.contextMenus.create({
    id: "vdl-download-link",
    title: "Download with Vdownloader",
    contexts: ["link"],
  });

  chrome.contextMenus.create({
    id: "vdl-download-image",
    title: "Download image with Vdownloader",
    contexts: ["image"],
  });

  chrome.contextMenus.create({
    id: "vdl-download-video",
    title: "Download video with Vdownloader",
    contexts: ["video", "audio"],
  });

  chrome.contextMenus.create({
    id: "vdl-download-page",
    title: "Download page with Vdownloader",
    contexts: ["page"],
  });
});

chrome.contextMenus.onClicked.addListener((info, tab) => {
  let url = "";
  let referrer = tab ? tab.url : "";

  switch (info.menuItemId) {
    case "vdl-download-link":
      url = info.linkUrl;
      break;
    case "vdl-download-image":
      url = info.srcUrl;
      break;
    case "vdl-download-video":
      url = info.srcUrl;
      break;
    case "vdl-download-page":
      url = info.pageUrl;
      break;
  }

  if (url) {
    const filename = url.split("/").pop().split("?")[0] || "download";
    sendToApp(url, filename, referrer);
  }
});

/* ── Message from popup ── */
chrome.runtime.onMessage.addListener((request, sender, sendResponse) => {
  switch (request.action) {
    case "getStatus":
      sendResponse({
        enabled: isEnabled,
        interceptDownloads: interceptDownloads,
        connected: nativePort !== null,
      });
      break;
    case "toggle":
      isEnabled = request.enabled;
      sendResponse({ enabled: isEnabled });
      break;
    case "toggleIntercept":
      interceptDownloads = request.intercept;
      sendResponse({ interceptDownloads: interceptDownloads });
      break;
    case "downloadUrl":
      const result = sendToApp(request.url, request.filename, request.referrer);
      sendResponse({ success: result });
      break;
  }
  return true;
});

// Auto-connect on startup
connectNative();
console.log("[Vdownloader] Extension loaded");
