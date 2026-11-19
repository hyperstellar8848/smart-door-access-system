
const CACHE_NAME = "smart-door-static-v1";

const STATIC_FILES = [
    "/style.css",
    "/script.js",
    "/manifest.json"
];

// Install service worker and cache static files.
self.addEventListener("install", (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME).then((cache) => {
            return cache.addAll(STATIC_FILES);
        })
    );

    self.skipWaiting();
});

// Activate and remove old caches.
self.addEventListener("activate", (event) => {
    event.waitUntil(
        caches.keys().then((cacheNames) => {
            return Promise.all(
                cacheNames
                    .filter((name) => name !== CACHE_NAME)
                    .map((name) => caches.delete(name))
            );
        })
    );

    self.clients.claim();
});

// Never cache authentication or API requests.
self.addEventListener("fetch", (event) => {
    const request = event.request;
    const url = new URL(request.url);

    if (
        request.method !== "GET" ||
        url.origin !== self.location.origin ||
        url.pathname.startsWith("/api/")
    ) {
        return;
    }

    event.respondWith(
        fetch(request)
            .then((response) => {
                if (response.ok) {
                    const copy = response.clone();

                    caches.open(CACHE_NAME).then((cache) => {
                        cache.put(request, copy);
                    });
                }

                return response;
            })
            .catch(() => caches.match(request))
    );
});
