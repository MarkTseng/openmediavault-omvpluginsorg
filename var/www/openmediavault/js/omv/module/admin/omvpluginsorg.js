/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Marcel Beck <marcel.beck@mbeck.org>
 * @author		Ian Moore <imooreyahoo@gmail.com>
 * @copyright Copyright (c) 2011-2012 Marcel Beck
 * @copyright Copyright (c) 2011 Ian Moore
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/NavigationPanel.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/plugins/FieldInfo.js")

Ext.ns("OMV.Module.System");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("system", "OmvPluginsOrg", {
	text: "OMV-Plugins.org",
	icon: "images/omvpluginsorg.png"
});

/**
 * @class OMV.Module.System.OmvPluginsOrg
 * @derived OMV.FormPanelExt
 */
OMV.Module.System.OmvPluginsOrg = function(config) {
	var initialConfig = {
		rpcService: "OmvPluginsOrg"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.System.OmvPluginsOrg.superclass.constructor.call(this,
	  initialConfig);
};
Ext.extend(OMV.Module.System.OmvPluginsOrg, OMV.FormPanelExt, {
	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General settings",
			defaults: {
				labelSeparator: ""
			},
			items: [{
				xtype: "checkbox",
				name: "use-stable",
				fieldLabel: "Stable",
				checked: false,
				boxLabel: "Use the list of plugins provided in the Stable repository.",
				inputValue: 1
			},{
				xtype: "checkbox",
				name: "use-unstable",
				fieldLabel: "Unstable",
				checked: false,
				boxLabel: "Use the list of plugins provided in the Unstable repository.",
				inputValue: 1
			},{
				fieldLabel: " ",
                html: "If, a stable and unstable package candidate exists, the package with a higher version number will be installed."
			}]
		}];
	}
});
OMV.NavigationPanelMgr.registerPanel("system", "OmvPluginsOrg", {
	cls: OMV.Module.System.OmvPluginsOrg
});

