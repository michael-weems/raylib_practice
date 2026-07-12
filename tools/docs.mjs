import { createServer } from "node:http";
import { cpSync, mkdirSync, readFileSync, statSync, writeFileSync } from "node:fs";
import { extname, join, normalize, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

let marked;
try {
  ({ marked } = await import("marked"));
} catch (_) {
  console.error("Documentation dependencies are missing. Run: npm ci");
  process.exit(1);
}

const scriptDirectory = fileURLToPath(new URL(".", import.meta.url));
const rootDirectory = resolve(scriptDirectory, "..");
const sourcePath = join(rootDirectory, "REPORT.md");
const outputDirectory = join(rootDirectory, "out", "docs");
const mermaidSource = join(
  rootDirectory,
  "node_modules",
  "mermaid",
  "dist"
);

function escapeHtml(value) {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function pageTemplate(content, sourceModified) {
  return `<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="description" content="Architecture and graphics guide for the software-rendered cube field">
  <title>Software-Rendered Cube Field &mdash; Project Report</title>
  <style>
    :root {
      color-scheme: dark;
      --background: #11151d;
      --panel: #181e29;
      --panel-raised: #202837;
      --text: #e8edf5;
      --muted: #aeb9c9;
      --rule: #344052;
      --accent: #70a5ff;
      --accent-soft: #263b5d;
      --code: #0d1118;
      --good: #72d19c;
      --warning: #ffd27a;
    }
    * { box-sizing: border-box; }
    html { scroll-behavior: smooth; scroll-padding-top: 1.5rem; }
    body {
      margin: 0;
      background: var(--background);
      color: var(--text);
      font: 17px/1.68 system-ui, -apple-system, "Segoe UI", sans-serif;
    }
    .layout { display: grid; grid-template-columns: minmax(15rem, 20rem) minmax(0, 1fr); }
    nav {
      position: sticky;
      top: 0;
      height: 100vh;
      overflow: auto;
      padding: 1.4rem 1.2rem 2rem;
      border-right: 1px solid var(--rule);
      background: var(--panel);
    }
    nav .title { margin-bottom: .8rem; color: var(--text); font-weight: 750; }
    nav a {
      display: block;
      padding: .22rem .35rem;
      border-radius: .3rem;
      color: var(--muted);
      font-size: .88rem;
      line-height: 1.35;
      text-decoration: none;
    }
    nav a[data-level="3"] { padding-left: 1.15rem; font-size: .82rem; }
    nav a:hover { color: var(--text); background: var(--accent-soft); }
    main { width: min(100%, 78rem); padding: 3.5rem clamp(1.25rem, 5vw, 5rem) 8rem; }
    h1 { margin: 0 0 .4rem; font-size: clamp(2.1rem, 5vw, 4rem); line-height: 1.08; }
    h2 { margin-top: 4rem; padding-top: .5rem; border-top: 1px solid var(--rule); font-size: 2rem; }
    h3 { margin-top: 2.4rem; font-size: 1.38rem; }
    h4 { margin-top: 1.8rem; font-size: 1.08rem; color: var(--good); }
    h1, h2, h3, h4 { line-height: 1.22; }
    p, li { max-width: 76ch; }
    a { color: var(--accent); }
    strong { color: #fff; }
    blockquote {
      margin: 1.5rem 0;
      padding: .7rem 1.2rem;
      border-left: .3rem solid var(--accent);
      background: var(--panel);
      color: var(--muted);
    }
    code {
      padding: .12rem .34rem;
      border: 1px solid #303949;
      border-radius: .3rem;
      background: var(--code);
      font: .9em/1.5 "Cascadia Code", "Fira Code", Consolas, monospace;
    }
    pre {
      max-width: 100%;
      overflow: auto;
      padding: 1rem 1.15rem;
      border: 1px solid var(--rule);
      border-radius: .55rem;
      background: var(--code);
    }
    pre code { padding: 0; border: 0; background: transparent; }
    .mermaid {
      overflow: auto;
      padding: 1.4rem;
      border: 1px solid var(--rule);
      border-radius: .55rem;
      background: #f6f8fb;
      text-align: center;
    }
    table { display: block; max-width: 100%; overflow: auto; border-collapse: collapse; }
    th, td { padding: .62rem .8rem; border: 1px solid var(--rule); text-align: left; vertical-align: top; }
    th { background: var(--panel-raised); }
    tr:nth-child(even) td { background: rgba(255, 255, 255, .018); }
    hr { margin: 3rem 0; border: 0; border-top: 1px solid var(--rule); }
    .document-meta { color: var(--muted); font-size: .84rem; }
    @media (max-width: 900px) {
      .layout { display: block; }
      nav { position: relative; width: 100%; height: auto; max-height: 45vh; border-right: 0; border-bottom: 1px solid var(--rule); }
      main { padding-top: 2rem; }
    }
    @media print {
      :root { color-scheme: light; --background: #fff; --text: #111; --muted: #444; --rule: #bbb; --code: #f5f5f5; }
      nav { display: none; }
      .layout { display: block; }
      main { width: 100%; padding: 0; }
      h2 { break-before: page; }
      a { color: inherit; }
    }
  </style>
</head>
<body>
  <div class="layout">
    <nav aria-label="Generated section navigation">
      <div class="title">Project report</div>
      <div id="section-navigation"></div>
    </nav>
    <main id="document">
      ${content}
      <p class="document-meta">Generated from REPORT.md. Source timestamp: ${escapeHtml(sourceModified)}</p>
    </main>
  </div>
  <script>
    function slugify(text) {
      return text.toLowerCase().trim()
        .replace(/[^a-z0-9\\s-]/g, "")
        .replace(/\\s+/g, "-")
        .replace(/-+/g, "-");
    }

    const usedIds = new Map();
    const navigation = document.getElementById("section-navigation");
    document.querySelectorAll("main h2, main h3").forEach((heading) => {
      const base = slugify(heading.textContent) || "section";
      const occurrence = usedIds.get(base) || 0;
      usedIds.set(base, occurrence + 1);
      heading.id = occurrence === 0 ? base : base + "-" + (occurrence + 1);
      const link = document.createElement("a");
      link.href = "#" + heading.id;
      link.dataset.level = heading.tagName.slice(1);
      link.textContent = heading.textContent;
      navigation.appendChild(link);
    });

    document.querySelectorAll("pre > code.language-mermaid").forEach((code) => {
      const diagram = document.createElement("pre");
      diagram.className = "mermaid";
      diagram.textContent = code.textContent;
      code.parentElement.replaceWith(diagram);
    });

    let documentationVersion = ${JSON.stringify(sourceModified)};
    setInterval(async () => {
      try {
        const response = await fetch("/__docs_version", { cache: "no-store" });
        const nextVersion = await response.text();
        if (nextVersion !== documentationVersion) location.reload();
      } catch (_) {
        // The development server may be restarting; the next poll will retry.
      }
    }, 1200);
  </script>
  <script type="module">
    import mermaid from "./vendor/mermaid/mermaid.esm.min.mjs";
    mermaid.initialize({
      startOnLoad: false,
      securityLevel: "strict",
      theme: "neutral",
      flowchart: { useMaxWidth: true },
      sequence: { useMaxWidth: true }
    });
    await mermaid.run({ querySelector: ".mermaid" });
  </script>
</body>
</html>`;
}

function buildDocumentation() {
  const markdown = readFileSync(sourcePath, "utf8");
  const modified = statSync(sourcePath).mtime.toISOString();
  marked.setOptions({ gfm: true });
  const content = marked.parse(markdown);
  mkdirSync(outputDirectory, { recursive: true });
  cpSync(mermaidSource, join(outputDirectory, "vendor", "mermaid"), {
    recursive: true,
    force: true
  });
  writeFileSync(
    join(outputDirectory, "index.html"),
    pageTemplate(content, modified),
    "utf8"
  );
  return modified;
}

const contentTypes = {
  ".css": "text/css; charset=utf-8",
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".map": "application/json; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".svg": "image/svg+xml"
};

function serveDocumentation() {
  let version = buildDocumentation();
  const portArgument = process.argv.find((argument) => argument.startsWith("--port="));
  const port = Number(portArgument?.slice(7) || process.env.DOCS_PORT || 4173);
  if (!Number.isInteger(port) || port < 1 || port > 65535) {
    throw new Error(`Invalid documentation server port: ${port}`);
  }

  const server = createServer((request, response) => {
    try {
      const sourceVersion = statSync(sourcePath).mtime.toISOString();
      if (sourceVersion !== version) version = buildDocumentation();

      if (request.url === "/__docs_version") {
        response.writeHead(200, {
          "Cache-Control": "no-store",
          "Content-Type": "text/plain; charset=utf-8"
        });
        response.end(version);
        return;
      }

      const requestPath = request.url === "/" ? "/index.html" : request.url;
      const decodedPath = decodeURIComponent(requestPath.split("?")[0]);
      const filePath = normalize(join(outputDirectory, decodedPath));
      const relativePath = relative(outputDirectory, filePath);
      if (relativePath.startsWith("..") || resolve(filePath) === resolve(outputDirectory)) {
        response.writeHead(403).end("Forbidden");
        return;
      }
      const file = readFileSync(filePath);
      response.writeHead(200, {
        "Cache-Control": "no-cache",
        "Content-Type": contentTypes[extname(filePath)] || "application/octet-stream"
      });
      response.end(file);
    } catch (error) {
      response.writeHead(error?.code === "ENOENT" ? 404 : 500, {
        "Content-Type": "text/plain; charset=utf-8"
      });
      response.end(error?.code === "ENOENT" ? "Not found" : String(error));
    }
  });

  server.listen(port, "127.0.0.1", () => {
    console.log(`Documentation server: http://127.0.0.1:${port}`);
    console.log("REPORT.md changes rebuild the page and reload the browser.");
  });
}

if (process.argv[2] === "build") {
  const version = buildDocumentation();
  console.log(`Generated out/docs/index.html from REPORT.md (${version})`);
} else if (process.argv[2] === "serve") {
  serveDocumentation();
} else {
  console.error("Usage: node tools/docs.mjs <build|serve> [--port=4173]");
  process.exitCode = 1;
}

