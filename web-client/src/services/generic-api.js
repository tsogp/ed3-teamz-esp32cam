import { DEFAULT_URL } from "./types";

class ApiCaller {
  constructor(baseUrl = "") {
    this.baseUrl = baseUrl;
    this.webSocket = null;
    this.wsListeners = {};
  }

  async request(endpoint, method = "GET", data = null, headers = {}) {
    const url = `${this.baseUrl}${endpoint}`;
    const options = {
      method,
      headers: {
        ...(method === "OPTIONS" ? { "Content-Type": "application/json" } : {}),
        ...headers,
      },
    };

    if (data) {
      options.body = JSON.stringify(data);
    }

    try {
      const response = await fetch(url, options);

      if (!response.ok) {
        if (response.status === 405) {
          await this.options(endpoint);
          return await this.request(endpoint, method, data, headers);
        }

        throw new Error(`HTTP error! Status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error(`Error in API call: ${error.message}`);
      throw error;
    }
  }

  async get(endpoint, headers = {}) {
    return this.request(endpoint, "GET", null, headers);
  }

  async post(endpoint, data, headers = {}) {
    return this.request(endpoint, "POST", data, headers);
  }

  async options(endpoint) {
    return this.request(endpoint, "OPTIONS", null, {});
  }

  connectWebSocket(wsEndpoint) {
    const url = `${this.baseUrl.replace(/^http/, "ws")}${wsEndpoint}`;
    this.webSocket = new WebSocket(url);

    this.webSocket.onopen = () => {
      console.log("WebSocket connected");
      this.emitWsEvent("open", {});
    };

    this.webSocket.onmessage = (event) => {
      try {
        const data = event.data; 
        this.emitWsEvent("message", data);
      } catch (error) {
        console.error("WebSocket message parsing error:", error);
        this.emitWsEvent("error", { error, rawData: event.data });
      }
    };

    this.webSocket.onerror = (error) => {
      console.error("WebSocket error:", error);
      this.emitWsEvent("error", error);
    };

    this.webSocket.onclose = (event) => {
      console.log("WebSocket connection closed:", event);
      this.emitWsEvent("close", event);
    };
  }

  sendWebSocketMessage(message) {
    if (!this.webSocket || this.webSocket.readyState !== WebSocket.OPEN) {
      throw new Error("WebSocket is not connected");
    }

    try {
      const data =
        typeof message === "string"
          ? JSON.stringify(JSON.parse(message))
          : JSON.stringify(message);
      console.log("Sending JSON message via WebSocket:", data);
      this.webSocket.send(data);
    } catch (error) {
      console.error("Error preparing WebSocket message:", error);
      throw new Error(
        "Invalid message format. Must be a valid JSON object or stringified JSON."
      );
    }
  }

  closeWebSocket() {
    if (this.webSocket) {
      this.webSocket.close();
      this.webSocket = null;
    }
  }

  addWsListener(eventType, callback) {
    if (!this.wsListeners[eventType]) {
      this.wsListeners[eventType] = [];
    }
    this.wsListeners[eventType].push(callback);
  }

  removeWsListener(eventType, callback) {
    if (this.wsListeners[eventType]) {
      this.wsListeners[eventType] = this.wsListeners[eventType].filter(
        (cb) => cb !== callback
      );
    }
  }

  emitWsEvent(eventType, data) {
    if (this.wsListeners[eventType]) {
      this.wsListeners[eventType].forEach((callback) => callback(data));
    }
  }
}

const api = new ApiCaller(DEFAULT_URL);

export default api;
